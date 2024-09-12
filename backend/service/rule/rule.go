package rule

import (
	"backend/global"
	"backend/models"
	"errors"
	"fmt"

	"github.com/mdlayher/netlink"
	"gorm.io/gorm"
)

type RuleService struct{}

type DelRuleReq struct {
	Index uint32 `json:"id" binding:"required,gte=1"`
}

type ModifyRuleReq struct {
	MID uint32 `json:"id" binding:"required,gte=1"`
	models.Rule
}

func (r *RuleService) GetRules() ([]models.Rule, error) {
	var rules []models.Rule
	err := global.FW_DB.Table("rules").Find(&rules).Error
	if err != nil {
		global.FW_LOG.Error("数据库查询规则失败")
		return nil, errors.New("数据库查询规则失败")
	}
	return rules, nil
}

func (r *RuleService) AddRule(rule *models.Rule) error {
	err := global.FW_DB.Transaction(func(db *gorm.DB) error {
		err := db.Table("rules").Create(rule).Error
		if err != nil {
			global.FW_LOG.Error("数据库添加规则失败")
			return errors.New("数据库添加规则失败")
		}
		mod_req := global.Request{}
		mod_req.CreateAddRuleReq(rule)
		global.FW_LOG.Info(fmt.Sprint(mod_req.Data))
		resps, err := global.Con.Cmd.Execute(netlink.Message{Data: mod_req.Data[:]})
		if err != nil {
			global.FW_LOG.Error("netlink出错: " + err.Error())
		}
		mod_resp := global.Response{}
		for _, resp := range resps {
			mod_resp.ParseRespone(resp.Data)
			if mod_resp.Type == global.FW_TYPE_OK {
				return nil
			}
			if mod_resp.Type == global.FW_TYPE_ERROR {
				global.FW_LOG.Error("内核模块添加规则失败")
				return errors.New("内核模块添加规则失败")
			}
		}
		global.FW_LOG.Error("内核模块添加规则失败")
		return errors.New("内核模块添加规则失败")
	})
	if err != nil {
		return err
	}
	global.FW_LOG.Info(fmt.Sprintf("添加规则成功, 规则ID:%d ", rule.ID))
	return nil
}

func (r *RuleService) DelRule(index uint32) error {
	err := global.FW_DB.Transaction(func(db *gorm.DB) error {
		if err := global.FW_DB.Table("rules").Where("id = ?", index).Delete(&models.Rule{}).Error; err != nil {
			global.FW_LOG.Error("数据库删除规则失败")
			return errors.New("数据库删除规则失败")
		}
		mod_req := global.Request{}
		mod_req.CreateDelRuleReq(index)
		resps, err := global.Con.Cmd.Execute(netlink.Message{Data: mod_req.Data[:]})
		if err != nil {
			global.FW_LOG.Error("netlink出错: " + err.Error())
		}
		mod_resp := global.Response{}
		for _, resp := range resps {
			mod_resp.ParseRespone(resp.Data)
			if mod_resp.Type == global.FW_TYPE_OK {
				return nil
			}
			if mod_resp.Type == global.FW_TYPE_ERROR {
				global.FW_LOG.Error(mod_resp.Info)
				return nil
			}
		}
		global.FW_LOG.Error("内核模块删除规则失败")
		return errors.New("内核模块删除规则失败")
	})
	if err != nil {
		return err
	}
	global.FW_LOG.Info(fmt.Sprintf("删除规则成功, 规则ID:%d ", index))
	return nil
}

func (r *RuleService) ModifyRule(rule *ModifyRuleReq) error {
	rule.ID.ID = rule.MID
	err := global.FW_DB.Transaction(func(db *gorm.DB) error {
		if err := global.FW_DB.Table("rules").Where("id = ?", rule.ID.ID).Updates(&rule.Rule).Error; err != nil {
			global.FW_LOG.Error("数据库修改规则失败")
			return errors.New("数据库修改规则失败")
		}
		mod_req := global.Request{}
		mod_req.CreateDelRuleReq(rule.ID.ID)
		resps, err := global.Con.Cmd.Execute(netlink.Message{Data: mod_req.Data[:]})
		if err != nil {
			global.FW_LOG.Error("netlink出错: " + err.Error())
		}
		mod_resp := global.Response{}
		var f bool
		for _, resp := range resps {
			mod_resp.ParseRespone(resp.Data)
			if mod_resp.Type == global.FW_TYPE_OK {
				f = true
				break
			}
			if mod_resp.Type == global.FW_TYPE_ERROR {
				global.FW_LOG.Error(mod_resp.Info)
				f = true
				break
			}
		}
		if !f {
			global.FW_LOG.Error("内核模块删除规则失败")
			return errors.New("内核模块删除规则失败")
		}
		mod_req.CreateAddRuleReq(&rule.Rule)
		resps, err = global.Con.Cmd.Execute(netlink.Message{Data: mod_req.Data[:]})
		if err != nil {
			global.FW_LOG.Error("netlink出错: " + err.Error())
		}
		for _, resp := range resps {
			mod_resp.ParseRespone(resp.Data)
			if mod_resp.Type == global.FW_TYPE_OK {
				return nil
			}
			if mod_resp.Type == global.FW_TYPE_ERROR {
				global.FW_LOG.Error(mod_resp.Info)
				break
			}
		}
		global.FW_LOG.Error("内核模块添加规则失败")
		return errors.New("内核模块添加规则失败")
	})
	if err != nil {
		return err
	}
	global.FW_LOG.Info(fmt.Sprintf("修改规则成功, 规则ID:%d ", rule.ID))
	return nil
}
