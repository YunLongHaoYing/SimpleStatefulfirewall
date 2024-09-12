package router

import "github.com/gin-gonic/gin"

type LogRouter struct{}

func (l *LogRouter) InitLogRouter(Router *gin.RouterGroup) {
	logRouter := Router.Group("log")
	{
		logRouter.GET("readmlog/:page", LogApi.ReadMlog)
		logRouter.GET("readplog/:page", LogApi.ReadPlog)
	}
}
