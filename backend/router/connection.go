package router

import "github.com/gin-gonic/gin"

type ConnectionRouter struct{}

func (c *ConnectionRouter) InitConnectionRouter(Router *gin.RouterGroup) {
	connectionRouter := Router.Group("connection")
	{
		connectionRouter.GET("get", ConnectionApi.GetConnection)
	}
}
