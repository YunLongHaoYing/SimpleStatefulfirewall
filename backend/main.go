package main

import (
	"backend/global"
	"backend/initialize"
	"backend/service/netlink"
)

const (
	NETLINK_CMD = 31
	NETLINK_LOG = 17
)

func main() {
	initialize.InitConfig()

	global.FW_LOG = initialize.InitLogger()
	defer global.FW_LOG.Sync()

	global.FW_DB = initialize.InitDB()
	if err := netlink.InitNetlink(); err != nil {
		return
	}
	defer global.Con.Cmd.Close()
	defer global.Con.Log.Close()

	if err := netlink.InitMod(); err != nil {
		return
	}

	go netlink.ConRecvLog()

	initialize.Run()
}
