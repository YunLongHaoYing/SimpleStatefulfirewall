package initialize

import (
	"fmt"
	"reflect"
	"strings"

	"github.com/gin-gonic/gin/binding"
	"github.com/go-playground/locales/en"
	"github.com/go-playground/locales/zh"
	ut "github.com/go-playground/universal-translator"
	"github.com/go-playground/validator/v10"
	entranslations "github.com/go-playground/validator/v10/translations/en"
	zhtranslations "github.com/go-playground/validator/v10/translations/zh"
)

// 验证器
var (
	trans ut.Translator
)

/****************************** 翻译器、自定义的验证器的初始化 ****************************/

// InitChineseTranslator 初始化中文翻译器
func InitChineseTranslator() {
	if err := initTrans("zh"); err != nil {
		panic("初始化翻译器错误" + err.Error())
	} else {
		fmt.Println("初始化中文翻译器成功")
	}
}

// InitEnglishTranslator 初始化英文翻译器
func InitEnglishTranslator() {
	if err := initTrans("en"); err != nil {
		panic("初始化翻译器错误" + err.Error())
	} else {
		fmt.Println("初始化英文翻译器成功")
	}
}

// initTrans 初始化翻译器
func initTrans(locale string) (err error) {
	//修改gin框架中的validator引擎属性, 实现定制
	if v, ok := binding.Validator.Engine().(*validator.Validate); ok {
		//注册一个获取json的tag的自定义方法
		v.RegisterTagNameFunc(func(fld reflect.StructField) string {
			name := strings.SplitN(fld.Tag.Get("json"), ",", 2)[0]
			if name == "-" {
				return ""
			}
			return name
		})

		zhT := zh.New() //中文翻译器
		enT := en.New() //英文翻译器
		//第一个参数是备用的语言环境，后面的参数是应该支持的语言环境
		uni := ut.New(enT, zhT, enT)
		// 根据输入获取传入指定的翻译器
		trans, ok = uni.GetTranslator(locale)
		if !ok {
			return fmt.Errorf("uni.GetTranslator(%s)", locale)
		}
		switch locale {
		case "en":
			err = entranslations.RegisterDefaultTranslations(v, trans)
		case "zh":
			err = zhtranslations.RegisterDefaultTranslations(v, trans)
		default:
			err = entranslations.RegisterDefaultTranslations(v, trans)
		}
	}
	return
}
