package models

type Rule struct {
	ID
	Timestamps
	SoftDeletes
	SAddr    string `json:"s_addr" gorm:"type:varchar(15);not null" binding:"required,ip"`
	DAddr    string `json:"d_addr" gorm:"type:varchar(15);not null" binding:"required,ip"`
	SMask    string `json:"s_mask" gorm:"type:varchar(15);not null" binding:"required,ip"`
	DMask    string `json:"d_mask" gorm:"type:varchar(15);not null" binding:"required,ip"`
	SPortMin uint16 `json:"s_port_min" binding:"gte=1,lte=65535"`
	SPortMax uint16 `json:"s_port_max" binding:"gte=1,lte=65535"`
	DPortMin uint16 `json:"d_port_min" binding:"gte=1,lte=65535"`
	DPortMax uint16 `json:"d_port_max" binding:"gte=1,lte=65535"`
	Protocol uint8  `json:"protocol" gorm:"not null" binding:"oneof=1 6 17"`
	Action   uint32 `json:"action" gorm:"not null" binding:"oneof=0 1"`
	Logging  uint8  `json:"logging" gorm:"not null" binding:"oneof=0 1"`
	Timeout  string `json:"timeout" gorm:"not null" binding:"required,datetime=2006-01-02 15:04:05"`
}
