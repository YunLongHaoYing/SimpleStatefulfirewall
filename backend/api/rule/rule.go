package rule

import (
	"backend/common/response"
	"backend/global"
	"backend/models"
	"backend/service"
	"backend/service/rule"

	"github.com/gin-gonic/gin"
)

type RuleApi struct{}

var (
	RuleService = service.ServiceApp.RuleService
)

func (r *RuleApi) GetRules(c *gin.Context) {
	rules, err := RuleService.GetRules()
	if err != nil {
		response.BusinessFail(c, err)
		global.AddMlog("读取过滤规则", 0, err.Error())
		return
	}
	global.AddMlog("读取过滤规则", 1, "")
	response.Success(c, rules)
}

func (r *RuleApi) CreateRule(c *gin.Context) {
	var rule models.Rule
	var err error
	if err = c.ShouldBind(&rule); err != nil {
		response.ValidateFail(c, response.NewValidatorError(err))
		return
	}
	if err = RuleService.AddRule(&rule); err != nil {
		global.AddMlog("创建过滤规则", 0, err.Error())
		response.BusinessFail(c, err)
		return
	}
	global.AddMlog("创建过滤规则", 1, "")
	response.Success(c, nil)
}

func (r *RuleApi) DeleteRule(c *gin.Context) {
	var err error
	var req rule.DelRuleReq
	if err = c.ShouldBind(&req); err != nil {
		response.ValidateFail(c, response.NewValidatorError(err))
		return
	}
	if err = RuleService.DelRule(req.Index); err != nil {
		response.BusinessFail(c, err)
		global.AddMlog("删除过滤规则", 0, err.Error())
		return
	}
	global.AddMlog("删除过滤规则", 1, "")
	response.Success(c, nil)
}

func (r *RuleApi) ModifyRule(c *gin.Context) {
	var rule rule.ModifyRuleReq
	var err error
	if err = c.ShouldBind(&rule); err != nil {
		response.ValidateFail(c, response.NewValidatorError(err))
		return
	}
	if err = RuleService.ModifyRule(&rule); err != nil {
		global.AddMlog("删除过滤规则", 0, err.Error())
		response.BusinessFail(c, err)
		return
	}
	global.AddMlog("删除过滤规则", 1, "")
	response.Success(c, nil)
}
