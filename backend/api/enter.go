package api

import (
	"backend/api/connection"
	"backend/api/log"
	"backend/api/rule"
	"backend/api/setting"
)

type Api struct {
	rule.RuleApi
	setting.DefActApi
	log.LogApi
	connection.ConnectionApi
}

var ApiApp = new(Api)
