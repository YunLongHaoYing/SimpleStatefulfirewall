package log

import (
	"backend/common/response"
	"backend/service"
	"backend/service/log"

	"github.com/gin-gonic/gin"
)

var LogService = service.ServiceApp.LogService

type LogApi struct{}

func (l *LogApi) ReadMlog(c *gin.Context) {
	var req log.ReqMlog
	var err error
	var res *log.RetMlog
	if err = c.ShouldBindUri(&req); err != nil {
		response.ValidateFail(c, response.NewValidatorError(err))
		return
	}
	if res, err = LogService.ReadMlog(&req); err != nil {
		response.BusinessFail(c, err.Error())
		return
	}
	response.Success(c, res)
}

func (l *LogApi) ReadPlog(c *gin.Context) {
	var req log.ReqPlog
	var err error
	var res *log.RetPlog
	if err = c.ShouldBindUri(&req); err != nil {
		response.ValidateFail(c, response.NewValidatorError(err))
		return
	}
	if res, err = LogService.ReadPlog(&req); err != nil {
		response.BusinessFail(c, err.Error())
		return
	}
	response.Success(c, res)
}
