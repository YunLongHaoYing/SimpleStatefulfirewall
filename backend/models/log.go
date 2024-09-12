package models

type MLog struct {
	ID
	Operation string `json:"operation"`
	Result    uint32 `json:"result"`
	ErrMsg    string `json:"err_msg"`
	Timestamps
}

type PLog struct {
	ID
	Time     string `json:"time"`
	Saddr    string `json:"saddr"`
	Daddr    string `json:"daddr"`
	Protocol uint8  `json:"protocol"`
	Sport    uint16 `json:"sport"`
	Dport    uint16 `json:"dport"`
	State    uint8  `json:"state"`
	Type     uint8  `json:"type"`
	Code     uint8  `json:"code"`
	Len      uint16 `json:"len"`
	Action   uint32 `json:"action"`
}
