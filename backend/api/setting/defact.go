package setting

import (
	"backend/common/response"
	"backend/global"
	"backend/service"
	"backend/service/setting"

	"github.com/gin-gonic/gin"
)

type DefActApi struct{}

var (
	DefActService = service.ServiceApp.DefActService
)

func (d *DefActApi) GetDefAct(c *gin.Context) {
	defAct, err := DefActService.GetDefAct()
	if err != nil {
		response.BusinessFail(c, err)
		global.AddMlog("读取默认操作", 0, err.Error())
		return
	}
	global.AddMlog("读取默认操作", 1, "")
	response.Success(c, defAct)
}

func (d *DefActApi) SetDefAct(c *gin.Context) {
	var defAct setting.DefAct
	var err error
	if err = c.ShouldBind(&defAct); err != nil {
		response.ValidateFail(c, response.NewValidatorError(err))
		return
	}
	if err = DefActService.SetDefAct(&defAct); err != nil {
		global.AddMlog("修改默认操作", 0, err.Error())
		response.BusinessFail(c, err)
		return
	}
	global.AddMlog("修改默认操作", 1, "")
	response.Success(c, nil)
}
