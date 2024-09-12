package connection

import (
	"backend/common/response"
	"backend/global"
	"backend/service"

	"github.com/gin-gonic/gin"
)

type ConnectionApi struct{}

var ConnectionService = service.ServiceApp.ConnectionService

func (co *ConnectionApi) GetConnection(c *gin.Context) {
	global.AddMlog("读取连接信息", 1, "")
	response.Success(c, ConnectionService.GetConnection())
}
