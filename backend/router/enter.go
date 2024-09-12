package router

import (
	"backend/api"
)

type Router struct {
	RuleRouter
	DefActRouter
	LogRouter
	ConnectionRouter
}

var RouterApp = new(Router)

var (
	RuleApi       = api.ApiApp.RuleApi
	DefActApi     = api.ApiApp.DefActApi
	LogApi        = api.ApiApp.LogApi
	ConnectionApi = api.ApiApp.ConnectionApi
)
