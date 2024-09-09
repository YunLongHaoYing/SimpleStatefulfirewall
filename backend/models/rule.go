package models

type Rule struct {
	ID
	Timestamps
	SoftDeletes
	SAddr    string `json:"s_addr" gorm:"type:varchar(15);not null"`
	DAddr    string `json:"d_addr" gorm:"type:varchar(15);not null"`
	SMask    string `json:"s_mask" gorm:"type:varchar(15);not null"`
	DMask    string `json:"d_mask" gorm:"type:varchar(15);not null"`
	SPortMin uint16 `json:"s_port_min" gorm:"type:smallint;not null"`
	SPortMax uint16 `json:"s_port_max" gorm:"type:smallint;not null"`
	DPortMin uint16 `json:"d_port_min" gorm:"type:smallint;not null"`
	DPortMax uint16 `json:"d_port_max" gorm:"type:smallint;not null"`
	Protocol uint8  `json:"protocol" gorm:"type:tinyint;not null"`
	Action   uint32 `json:"action" gorm:"type:int;not null"`
	Logging  uint8  `json:"logging" gorm:"type:tinyint;not null"`
	Timeout  uint64 `json:"timeout" gorm:"type:bigint;not null"`
}
