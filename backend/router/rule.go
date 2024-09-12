package router

import "github.com/gin-gonic/gin"

type RuleRouter struct{}

func (r *RuleRouter) InitRuleRouter(Router *gin.RouterGroup) {
	ruleRouter := Router.Group("rule")
	{
		ruleRouter.GET("get", RuleApi.GetRules)
		ruleRouter.POST("create", RuleApi.CreateRule)
		ruleRouter.DELETE("delete", RuleApi.DeleteRule)
		ruleRouter.PUT("modify", RuleApi.ModifyRule)
	}
}
