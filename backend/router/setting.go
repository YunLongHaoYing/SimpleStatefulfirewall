package router

import "github.com/gin-gonic/gin"

type DefActRouter struct{}

func (d *DefActRouter) InitDefActRouter(Router *gin.RouterGroup) {
	defActRouter := Router.Group("setting")
	{
		defActRouter.GET("defact/get", DefActApi.GetDefAct)
		defActRouter.POST("defact/set", DefActApi.SetDefAct)
	}
}
