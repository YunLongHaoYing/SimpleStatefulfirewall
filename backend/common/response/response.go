package response

import (
	"backend/global"
	"net/http"
	"strings"

	"github.com/gin-gonic/gin"
	ut "github.com/go-playground/universal-translator"
	"github.com/go-playground/validator"
)

// 响应结构体
type Response struct {
	ErrorCode int         `json:"err"`  // 自定义错误码
	Data      interface{} `json:"data"` // 数据
	Message   interface{} `json:"msg"`  // 信息
}

// Success 响应成功 ErrorCode 为 0 表示成功
func Success(c *gin.Context, data interface{}) {
	c.JSON(http.StatusOK, Response{
		0,
		data,
		"ok",
	})
}

// Fail 响应失败 ErrorCode 不为 0 表示失败
func Fail(c *gin.Context, errorCode int, msg interface{}) {
	c.JSON(http.StatusOK, Response{
		errorCode,
		nil,
		msg,
	})
}

// ValidateFail 请求参数验证失败
func ValidateFail(c *gin.Context, msg interface{}) {
	Fail(c, global.Errors.ValidateError.ErrorCode, msg)
}

// BusinessFail 业务逻辑失败
func BusinessFail(c *gin.Context, msg interface{}) {
	Fail(c, global.Errors.BusinessError.ErrorCode, msg)
}

/****************************** 全局变量 ****************************/
var (
	// Trans 全局的翻译器
	Trans ut.Translator
)

/****************************** 辅助函数 ****************************/

// removeTopStruct 移除打印的错误信息中的结构体包前缀
func removeTopStruct(fields map[string]string) map[string]string {
	rsp := map[string]string{}
	for field, err := range fields {
		rsp[field[strings.Index(field, ".")+1:]] = strings.TrimPrefix(err, field[strings.Index(field, ".")+1:])
	}
	return rsp
}

func NewValidatorError(err error) interface{} {
	errs, ok := err.(validator.ValidationErrors)
	if !ok {
		return err.Error()
	}
	return removeTopStruct(errs.Translate(Trans))
}
