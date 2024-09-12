package service

import (
	"backend/service/connection"
	"backend/service/log"
	"backend/service/rule"
	"backend/service/setting"
)

type Service struct {
	rule.RuleService
	setting.DefActService
	log.LogService
	connection.ConnectionService
}

var ServiceApp = new(Service)
