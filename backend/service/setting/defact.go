package setting

import (
	"backend/global"
	"errors"

	"github.com/mdlayher/netlink"
)

type DefActService struct{}

type DefAct struct {
	Act uint32 `json:"default_act" binding:"oneof=0 1"`
}

func (d *DefActService) GetDefAct() (*DefAct, error) {
	var defact DefAct
	mod_req := global.Request{}
	mod_req.CreateReadDefaultActionReq()
	mod_resps, err := global.Con.Cmd.Execute(netlink.Message{Data: mod_req.Data[:]})
	if err != nil {
		global.FW_LOG.Error("netlink出错: " + err.Error())
		return nil, errors.New("netlink出错")
	}
	mod_resp := global.Response{}
	for _, resp := range mod_resps {
		mod_resp.ParseRespone(resp.Data)
		if mod_resp.Type == global.FW_TYPE_OK {
			defact.Act = mod_resp.DefAct
			return &defact, nil
		}
		if mod_resp.Type == global.FW_TYPE_ERROR {
			global.FW_LOG.Error("内核模块读取默认动作失败")
			return nil, errors.New("内核模块读取默认动作失败")
		}
	}
	global.FW_LOG.Error("内核模块读取默认动作失败")
	return nil, errors.New("内核模块读取默认动作失败")
}

func (d *DefActService) SetDefAct(defAct *DefAct) error {
	mod_req := global.Request{}
	mod_req.CreateSetDefaultActionReq(defAct.Act)
	mod_resps, err := global.Con.Cmd.Execute(netlink.Message{Data: mod_req.Data[:]})
	if err != nil {
		global.FW_LOG.Error("netlink出错: " + err.Error())
		return errors.New("netlink出错")
	}
	mod_resp := global.Response{}
	for _, resp := range mod_resps {
		mod_resp.ParseRespone(resp.Data)
		if mod_resp.Type == global.FW_TYPE_OK {
			return nil
		}
		if mod_resp.Type == global.FW_TYPE_ERROR {
			global.FW_LOG.Error("内核模块设置默认动作失败")
			return errors.New("内核模块设置默认动作失败")
		}
	}
	global.FW_LOG.Error("内核模块设置默认动作失败")
	return errors.New("内核模块设置默认动作失败")
}
