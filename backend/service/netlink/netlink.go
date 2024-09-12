package netlink

import (
	"backend/global"
	"backend/service/rule"
	"errors"
	"fmt"

	"github.com/mdlayher/netlink"
)

func InitNetlink() error {
	var err error
	global.Con.Cmd, err = netlink.Dial(global.NETLINK_CMD, nil)
	if err != nil {
		global.FW_LOG.Error(fmt.Sprintf("failed to dial netlink: %v", err))
		return errors.New("连接netlink失败")
	}

	global.Con.Log, err = netlink.Dial(global.NETLINK_LOG, nil)
	if err != nil {
		global.FW_LOG.Error(fmt.Sprintf("failed to dial netlink: %v", err))
		return errors.New("连接netlink失败")
	}
	return nil
}

func InitRule() error {
	var RuleService = new(rule.RuleService)
	rules, err := RuleService.GetRules()
	if err != nil {
		return err
	}
	for _, rule := range rules {
		mod_req := global.Request{}
		mod_req.CreateAddRuleReq(&rule)
		resps, err := global.Con.Cmd.Execute(netlink.Message{Data: mod_req.Data[:]})
		if err != nil {
			global.FW_LOG.Error("netlink出错: " + err.Error())
			return errors.New("netlink出错")
		}
		mod_resp := global.Response{}
		for _, resp := range resps {
			mod_resp.ParseRespone(resp.Data)
			if mod_resp.Type == global.FW_TYPE_OK {
				break
			}
			if mod_resp.Type == global.FW_TYPE_ERROR {
				global.FW_LOG.Error(fmt.Sprintf("内核模块添加规则失败, 规则ID:%d", rule.ID))
				break
			}
		}
	}
	return nil
}

func InitMod() error {
	req := netlink.Message{
		Data: make([]byte, 4),
	}
	global.Con.Cmd.Send(req)

	if err := InitRule(); err != nil {
		return err
	}
	return nil
}

func ConRecvLog() {
	for {
		msgs, err := global.Con.Log.Receive()
		if err != nil {
			global.FW_LOG.Error(fmt.Sprintf("failed to receive: %v", err))
			continue
		}
		var mod_resp global.Response
		for _, m := range msgs {
			global.FW_LOG.Info(fmt.Sprintf("received: %+v", m))
			mod_resp.ParseRespone(m.Data)
			if mod_resp.Type == global.FW_TYPE_LOG {
				global.AddPlog(mod_resp.Package)
			}
		}
	}
}
