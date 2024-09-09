package global

import (
	"github.com/mdlayher/netlink"
)

const (
	NETLINK_CMD = 31
	NETLINK_LOG = 17
)

var Con struct {
	Cmd *netlink.Conn
	Log *netlink.Conn
}
