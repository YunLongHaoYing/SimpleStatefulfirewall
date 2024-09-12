package global

import (
	"backend/models"
	"backend/utils"
	"encoding/binary"
	"time"
)

// request
const (
	FW_INIT = iota
	FW_ADDRULE
	FW_DELRULE
	FW_DEFACT
	FW_READDEFACT
	FW_ADDNAT
	FW_DELNAT
	FW_READNAT
)

type Request struct {
	Data [72]byte
}

func (r *Request) CreateAddRuleReq(rule *models.Rule) {
	r.Data[0] = byte(FW_ADDRULE)
	binary.BigEndian.PutUint32(r.Data[8:12], rule.ID.ID)
	binary.BigEndian.PutUint32(r.Data[12:16], utils.IpToUint32(rule.SAddr))
	binary.BigEndian.PutUint32(r.Data[16:20], utils.IpToUint32(rule.DAddr))
	binary.BigEndian.PutUint32(r.Data[20:24], utils.IpToUint32(rule.SMask))
	binary.BigEndian.PutUint32(r.Data[24:28], utils.IpToUint32(rule.DMask))
	binary.BigEndian.PutUint16(r.Data[28:30], rule.SPortMin)
	binary.BigEndian.PutUint16(r.Data[30:32], rule.SPortMax)
	binary.BigEndian.PutUint16(r.Data[32:34], rule.DPortMin)
	binary.BigEndian.PutUint16(r.Data[34:36], rule.DPortMax)
	r.Data[36] = rule.Protocol
	binary.BigEndian.PutUint32(r.Data[40:44], rule.Action)
	r.Data[44] = rule.Logging
	t, err := time.ParseInLocation("2006-01-02 15:04:05", rule.Timeout, time.Local)
	if err != nil {
		t = time.Now()
	}
	binary.BigEndian.PutUint64(r.Data[48:56], uint64(t.UnixNano()))
}

func (r *Request) CreateDelRuleReq(index uint32) {
	r.Data[0] = byte(FW_DELRULE)
	binary.BigEndian.PutUint32(r.Data[8:12], index)
}

func (r *Request) CreateSetDefaultActionReq(action uint32) {
	r.Data[0] = byte(FW_DEFACT)
	binary.BigEndian.PutUint32(r.Data[8:12], action)
}

func (r *Request) CreateReadDefaultActionReq() {
	r.Data[0] = byte(FW_READDEFACT)
}

// response
// netlink响应类型
const (
	FW_TYPE_OK = iota
	FW_TYPE_ERROR
	FW_TYPE_RULE
	FW_TYPE_LOG
	FW_TYPE_MLOG
	FW_TYPE_CONN
	FW_TYPE_NAT
)

type Tcp struct {
	Sport uint16 `gorm:"column:sport" json:"s_port"`
	Dport uint16 `gorm:"column:dport" json:"d_port"`
	State uint8  `gorm:"column:tcpstate" json:"tcp_state"`
}

type Udp struct {
	Sport uint16
	Dport uint16
}

type Icmp struct {
	Type uint8 `gorm:"column:icmptype" json:"icmp_type"`
	Code uint8 `gorm:"column:icmpcode" json:"icmp_code"`
}

type Package_log struct {
	Idx      uint32
	ts       uint64
	Saddr    uint32
	Daddr    uint32
	Protocol uint8
	Tcp
	Udp
	Icmp
	Len    uint16
	Action uint32
}

type Connection struct {
	Saddr    uint32
	Daddr    uint32
	Protocol uint8
	Tcp
	Udp
	Icmp
	Ts uint64
}
type Connection_set struct {
	Num        int
	Connection []Connection
}

var Conn_set = Connection_set{0, nil}

type Response struct {
	Type    uint8
	Len     uint32
	Package Package_log
	Info    string
	DefAct  uint32
}

func (r *Response) ParseRespone(data []byte) {
	r.Type = data[0]
	r.Len = binary.BigEndian.Uint32(data[4:8])
	if r.Type == FW_TYPE_LOG {
		r.Package.Idx = binary.BigEndian.Uint32(data[8:12])
		r.Package.ts = binary.BigEndian.Uint64(data[16:24])
		r.Package.Saddr = binary.BigEndian.Uint32(data[24:28])
		r.Package.Daddr = binary.BigEndian.Uint32(data[28:32])
		r.Package.Protocol = data[32]
		r.Package.Tcp.Sport = binary.BigEndian.Uint16(data[34:36])
		r.Package.Tcp.Dport = binary.BigEndian.Uint16(data[36:38])
		r.Package.Tcp.State = data[38]
		r.Package.Udp.Sport = binary.BigEndian.Uint16(data[34:36])
		r.Package.Udp.Dport = binary.BigEndian.Uint16(data[36:38])
		r.Package.Icmp.Type = data[34]
		r.Package.Icmp.Code = data[36]
		r.Package.Len = binary.BigEndian.Uint16(data[40:42])
		r.Package.Action = binary.BigEndian.Uint32(data[44:48])
	}
	if r.Type == FW_TYPE_OK || r.Type == FW_TYPE_ERROR {
		r.Info = string(data[8:])
		r.DefAct = binary.BigEndian.Uint32(data[8:12])
	}
	if r.Type == FW_TYPE_CONN {
		Conn_set.Num = int(float64(r.Len) / 40.0)
		Conn_set.Connection = nil
		for i := 0; i < Conn_set.Num; i++ {
			var Connection Connection
			Connection.Saddr = binary.BigEndian.Uint32(data[8+i*40 : 12+i*40])
			Connection.Daddr = binary.BigEndian.Uint32(data[12+i*40 : 16+i*40])
			Connection.Protocol = data[16+i*40]
			Connection.Ts = binary.BigEndian.Uint64(data[24+i*40 : 32+i*40])
			Connection.Tcp.Sport = binary.BigEndian.Uint16(data[18+i*40 : 20+i*40])
			Connection.Tcp.Dport = binary.BigEndian.Uint16(data[20+i*40 : 22+i*40])
			Connection.Tcp.State = data[22+i*40]
			Connection.Udp.Sport = binary.BigEndian.Uint16(data[18+i*40 : 20+i*40])
			Connection.Udp.Dport = binary.BigEndian.Uint16(data[20+i*40 : 22+i*40])
			Connection.Icmp.Type = data[18+i*40]
			Connection.Icmp.Code = data[20+i*40]
			Conn_set.Connection = append(Conn_set.Connection, Connection)
		}
	}

}
