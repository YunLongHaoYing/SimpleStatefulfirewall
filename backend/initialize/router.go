package initialize

import (
	"backend/global"
	"backend/router"

	"github.com/gin-gonic/gin"
)

func setupRouter() *gin.Engine {
	r := gin.Default()
	routergroup := r.Group("/myfw")
	router.RouterApp.InitRuleRouter(routergroup)
	router.RouterApp.InitDefActRouter(routergroup)
	router.RouterApp.InitLogRouter(routergroup)
	router.RouterApp.InitConnectionRouter(routergroup)
	return r
}

func Run() {
	router := setupRouter()
	router.Run(":" + global.App.Config.App.Port)
}
