package log

import (
	"backend/global"
	"backend/models"
	"errors"
)

type LogService struct{}

type ReqMlog struct {
	Page uint32 `json:"page" uri:"page" binding:"required,gte=1"`
}

type RetMlog struct {
	Total int64         `json:"total"`
	Data  []models.MLog `json:"data"`
}

func (l *LogService) ReadMlog(req *ReqMlog) (*RetMlog, error) {
	var ret RetMlog
	pagesize := global.App.Config.Database.PageSize
	offset := (req.Page - 1) * pagesize
	db := global.FW_DB.Model(&models.MLog{})

	err := db.Count(&ret.Total).Error
	if err != nil {
		global.FW_LOG.Error("读取管理日志错误")
		return nil, errors.New("读取管理日志错误")
	}

	err = db.Order("id DESC").Offset(int(offset)).Limit(int(pagesize)).Find(&ret.Data).Error
	if err != nil {
		global.FW_LOG.Error("读取管理日志错误")
		return nil, errors.New("读取管理日志错误")
	}
	return &ret, nil
}

type RetPlog struct {
	Total int64         `json:"total"`
	Data  []models.PLog `json:"data"`
}

type ReqPlog struct {
	Page       uint32  `json:"page" uri:"page" binding:"required,gte=1"`
	Protocol   []uint8 `json:"protocol" uri:"protocol"`
	TimeBefore string  `json:"time_before" uri:"time_before"`
	TimeAfter  string  `json:"time_after" uri:"time_after"`
}

func (l *LogService) ReadPlog(req *ReqPlog) (*RetPlog, error) {
	var ret RetPlog
	pagesize := global.App.Config.Database.PageSize
	offset := (req.Page - 1) * pagesize
	db := global.FW_DB.Model(&models.PLog{})

	err := db.Count(&ret.Total).Error
	if err != nil {
		global.FW_LOG.Error("读取流量日志错误")
		return nil, errors.New("读取流量日志错误")
	}

	if len(req.Protocol) > 0 {
		db = db.Where("protocol IN (?)", req.Protocol)
	}
	if req.TimeBefore != "" {
		db = db.Where("time < ?", req.TimeBefore)
	}
	if req.TimeAfter != "" {
		db = db.Where("time > ?", req.TimeAfter)
	}

	err = db.Order("id DESC").Offset(int(offset)).Limit(int(pagesize)).Find(&ret.Data).Error
	if err != nil {
		global.FW_LOG.Error("读取流量日志错误")
		return nil, errors.New("读取流量日志错误")
	}
	return &ret, nil
}
