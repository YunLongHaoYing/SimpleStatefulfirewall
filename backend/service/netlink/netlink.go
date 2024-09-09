package netlink

import (
	"backend/global"
	"errors"
	"fmt"

	"github.com/mdlayher/netlink"
)

// // 过滤的数据包
// type Package_log struct {
//     idx uint // unsigned int
//     ts uint64 // ktime_t
//     saddr uint32 // __be32
//     daddr uint32 // __be32
//     protocol uint8 // unsigned char
// 	icmp struct {
//             type uint8 // unsigned char
//             code uint8
//     }
//         struct {
//             __be16 sport;
//             __be16 dport;
//             __u8 state;
//         } tcp;
//         struct {
//             __be16 sport;
//             __be16 dport;
//         } udp;
//     }
//     __be16 len;    // unsigned short
//     __be32 action; // unsigned int
//     struct list_head node;
// };

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

func InitMod() {
	req := netlink.Message{
		Data: make([]byte, 4),
	}
	global.Con.Cmd.Send(req)
}

func ConRecvLog() {
	for {
		msgs, err := global.Con.Log.Receive()
		if err != nil {
			global.FW_LOG.Error(fmt.Sprintf("failed to receive: %v", err))
			continue
		}
		for _, m := range msgs {
			global.FW_LOG.Info(fmt.Sprintf("received: %+v", m))
		}
	}
}
