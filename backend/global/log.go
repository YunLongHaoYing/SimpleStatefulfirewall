package global

import (
	"backend/models"
	"backend/utils"
	"time"
)

func AddMlog(operation string, result uint32, err string) {
	var log = models.MLog{
		Operation: operation,
		Result:    result,
		ErrMsg:    err,
	}
	if err := FW_DB.Table("m_logs").Create(&log).Error; err != nil {
		FW_LOG.Error("添加管理日志错误")
	}
}

func AddPlog(plog Package_log) {
	time := time.UnixMicro(int64(plog.ts) / 1000).Format("2006-01-02 15:04:05")
	var sport, dport uint16
	if plog.Protocol == 6 {
		sport = plog.Tcp.Sport
		dport = plog.Tcp.Dport
	}
	if plog.Protocol == 17 {
		sport = plog.Udp.Sport
		dport = plog.Udp.Dport
	}
	var log = models.PLog{
		Time:     time,
		Saddr:    utils.Uint32ToIp(plog.Saddr),
		Daddr:    utils.Uint32ToIp(plog.Daddr),
		Protocol: plog.Protocol,
		Sport:    sport,
		Dport:    dport,
		State:    plog.State,
		Type:     plog.Type,
		Code:     plog.Code,
		Len:      plog.Len,
		Action:   plog.Action,
	}
	if err := FW_DB.Table("p_logs").Create(&log).Error; err != nil {
		FW_LOG.Error("添加流量日志日志错误")
	}
}
