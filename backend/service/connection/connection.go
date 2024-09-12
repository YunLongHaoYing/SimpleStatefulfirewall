package connection

import (
	"backend/global"
	"backend/utils"
	"time"
)

type ConnectionService struct{}

type Conn struct {
	Saddr    string `json:"saddr"`
	Daddr    string `json:"daddr"`
	Protocol uint8  `json:"protocol"`
	Sport    uint16 `json:"sport"`
	Dport    uint16 `json:"dport"`
	State    uint8  `json:"state"`
	Type     uint8  `json:"type"`
	Code     uint8  `json:"code"`
	Timeout  string `json:"timeout"`
}

type Conn_set struct {
	Num         int    `json:"num"`
	Connections []Conn `json:"connections"`
}

func (c *ConnectionService) GetConnection() *Conn_set {
	var conn_set Conn_set
	conn_set.Num = global.Conn_set.Num
	for _, conn := range global.Conn_set.Connection {
		var sport, dport uint16
		if conn.Protocol == 6 {
			sport = conn.Tcp.Sport
			dport = conn.Tcp.Dport
		}
		if conn.Protocol == 17 {
			sport = conn.Udp.Sport
			dport = conn.Udp.Dport
		}
		conn_set.Connections = append(conn_set.Connections, Conn{
			Saddr:    utils.Uint32ToIp(conn.Saddr),
			Daddr:    utils.Uint32ToIp(conn.Daddr),
			Protocol: conn.Protocol,
			Sport:    sport,
			Dport:    dport,
			State:    conn.State,
			Type:     conn.Type,
			Code:     conn.Code,
			Timeout:  time.UnixMicro(int64(conn.Ts) / 1000).Format("2006-01-02 15:04:05"),
		})
	}
	return &conn_set
}
