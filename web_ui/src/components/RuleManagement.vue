<template>
  <div>
    <el-button type="primary" @click="drawer = true;drawerAction = '新建规则'" plain>
      <el-icon><CirclePlus /></el-icon>
      <span>新建规则</span>
    </el-button>
    <el-button type="primary" @click="reFresh" plain>
      <el-icon><RefreshLeft /></el-icon>
      <span>刷新</span>
    </el-button>
    <el-button type="primary" @click="setDefAct" plain>
      <span>默认动作:{{defaultAction.action}}</span>
    </el-button>
  </div>
  <div>
    <el-table :data="ruleTableData"  style="width: 100%">
    <el-table-column fixed prop="id" label="规则ID" width="100" />
    <el-table-column prop="s_addr" label="源IP" width="135" />
    <el-table-column prop="d_addr" label="目的IP" width="135" />
    <el-table-column prop="s_mask" label="源IP掩码" width="135" />
    <el-table-column prop="d_mask" label="目的IP掩码" width="135" />
    <el-table-column prop="proto" label="协议" width="120" />
    <el-table-column prop="s_port" label="源端口" width="120" />
    <el-table-column prop="d_port" label="目的端口" width="120" />
    <el-table-column prop="act" label="动作" width="60" />
    <el-table-column prop="log" label="日志" width="60" />
    <el-table-column prop="timeout" label="超时时间" width="240" />
    <el-table-column fixed="right" label="操作" min-width="120">
      <template #="scope">
        <el-button type="primary" circle @click="drawer = true; drawerAction = '编辑规则'; oneRule = scope.row" >
          <el-icon><Edit /></el-icon>
        </el-button>
        <el-button type="danger" @click="delRule(scope.row.id)" circle >
          <el-icon><Delete /></el-icon>
        </el-button>
      </template>
    </el-table-column>
  </el-table>
  </div>
  <el-drawer
    v-model="drawer"
    :title="drawerAction"
    direction='rtl'>
    <el-form
      ref="ruleFormRef"
      style="max-width: 340px"
      :model="oneRule"
      :rules="rules"
      label-width="auto"
      class="demo-ruleForm"
      status-icon>
      <el-form-item label="源IP" prop="s_addr">
        <el-input v-model="oneRule.s_addr" style="max-width: 130px" />
      </el-form-item>
      <el-form-item label="源IP掩码" prop="s_mask">
        <el-input v-model="oneRule.s_mask" style="max-width: 130px" />
      </el-form-item>
      <el-form-item label="目的IP" prop="d_addr">
        <el-input v-model="oneRule.d_addr" style="max-width: 130px" />
      </el-form-item>
      <el-form-item label="目的IP掩码" prop="d_mask">
        <el-input v-model="oneRule.d_mask" style="max-width: 130px" />
      </el-form-item>
      <el-form-item label="协议" prop="protocol">
        <el-radio-group v-model="oneRule.protocol">
          <el-radio :value=6>Tcp</el-radio>
          <el-radio :value=17>Udp</el-radio>
          <el-radio :value=1>Icmp</el-radio>
        </el-radio-group>
      </el-form-item>
      <div v-show="oneRule.protocol !== 1">
        <el-form-item label="源端口">
          <el-input-number v-model="oneRule.s_port_min" style="max-width: 120px" />
          <span class="text-gray-500">-</span>
          <el-input-number v-model="oneRule.s_port_max" style="max-width: 120px" />
        </el-form-item>
        <el-form-item label="目的端口" >
          <el-input-number v-model="oneRule.d_port_min" style="max-width: 120px" />
          <span class="text-gray-500">-</span>
          <el-input-number v-model="oneRule.d_port_max" style="max-width: 120px" />
        </el-form-item>
      </div>
      <el-form-item label="动作" prop="action">
        <el-radio-group v-model="oneRule.action">
          <el-radio :value=1>接收</el-radio>
          <el-radio :value=0>丢弃</el-radio>
        </el-radio-group>
      </el-form-item>
      <el-form-item label="是否记录流量日志" prop="logging">
        <el-radio-group v-model="oneRule.logging">
          <el-radio :value=1>是</el-radio>
          <el-radio :value=0>否</el-radio>
        </el-radio-group>
      </el-form-item>
      <el-form-item label="过期时间" prop="timeout">
        <el-date-picker
        v-model="oneRule.timeout"
        type="datetime"
        placeholder="Select date and time" 
        value-format="YYYY-MM-DD hh:mm:ss"
        />
      </el-form-item>
    </el-form>
    <el-button type="primary" @click="submitRule" plain>
      <span>提交</span>
    </el-button>
  </el-drawer>
</template>

<script>
import { onMounted, reactive,ref} from "vue"
import { validateIP } from "@/utils/validator"
import { getRuleList, getDefAct, addRule, modifyRule, setDefAct, delRule } from "@/api/rule"

const drawer=ref(false)
const drawerAction=ref("")
const defaultAction=ref({
  action:"",
  default_act:0
})

const oneRule=ref({
  id:0,
  s_addr:"",
  d_addr:"",
  s_mask:"",
  d_mask:"",
  s_port:"",
  d_port:"",
  proto:"",
  act:"",
  log:"",
  s_port_min:0,
  s_port_max:0,
  d_port_min:0,
  d_port_max:0,
  protocol:1,
  action:0,
  logging:0,
  timeout:""
})

function RuleF() {
  this.id = 0
  this.s_addr = ""
  this.d_addr = ""
  this.s_mask = ""
  this.d_mask = ""
  this.s_port = ""
  this.d_port = ""
  this.s_port_min = 0
  this.s_port_max = 0
  this.d_port_min = 0
  this.d_port_max = 0
  this.proto = ""
  this.act = ""
  this.log = ""
  this.timeout = ""
  this.protocol = 1
  this.action = 0
  this.logging = 0
}

const rules = reactive(
  {
    s_addr:[
      {required: true, message: '请输入正确的IP地址', validator: validateIP, trigger: 'blur'}
    ],
    d_addr:[
      {required: true, message: '请输入正确的IP地址', validator: validateIP, trigger: 'blur'}
    ],
    s_mask:[
      {required: true, message: '请输入正确的IP掩码', validator: validateIP, trigger: 'blur'}
    ],
    d_mask:[
      {required: true, message: '请输入正确的IP掩码', validator: validateIP, trigger: 'blur'}
    ],
    protocol:[
      { required: true, message: '请选择一个协议', trigger: 'blur' },
      { type: 'enum', enum: [1, 6, 17], message: '必须是Tcp,Udp,Icmp三个协议之一', trigger: 'blur' },
    ],
    s_port_min:[
      {required: oneRule.value.protocol != 1, message: '请输入正确的最小源端口', trigger: 'blur'},
      { min: 1, max: oneRule.value.s_port_max, message: '端口范围错误', trigger: 'blur' },
    ],
    s_port_max:[
      {required: oneRule.value.protocol != 1, message: '请输入正确的最大源端口', trigger: 'blur'},
      { min: oneRule.value.s_port_min, max: 65535, message: '端口范围错误', trigger: 'blur' },
    ],
    d_port_min:[
      {required: oneRule.value.protocol != 1, message: '请输入正确的最小目的端口', trigger: 'blur'},
      { min: 1, max: oneRule.value.s_port_max, message: '端口范围错误', trigger: 'blur' },
    ],
    d_port_max:[
      {required: oneRule.value.protocol != 1, message: '请输入正确的最大源端口', trigger: 'blur'},
      { min: oneRule.value.s_port_min, max: 65535, message: '端口范围错误', trigger: 'blur' },
    ],
    timeout:[
      {required: true, message: '请选择一个过期时间', trigger: 'blur'},
      { type: 'date', message: '请输入正确的日期时间', trigger: 'blur' },
    ]
  }
)

const ruleTableData=ref(
  [
    {
      id:0,
      s_addr:"10.12.0.3",
    }
  ]
)

function refresh() {
  getRuleList().then(res => {
    ruleTableData.value = []
    for(let i=0;i<res.data.length;i++) {
      var r = new RuleF()
      r.id = res.data[i].id
      r.s_addr = res.data[i].s_addr
      r.d_addr = res.data[i].d_addr
      r.s_mask = res.data[i].s_mask
      r.d_mask = res.data[i].d_mask
      r.s_port = res.data[i].s_port_min + "-" + res.data[i].s_port_max
      r.d_port = res.data[i].d_port_min + "-" + res.data[i].d_port_max
      if(res.data[i].protocol == 1) {
        r.s_port = r.d_port = "-"
      }
      r.s_port_min = res.data[i].s_port_min
      r.s_port_max = res.data[i].s_port_max
      r.d_port_min = res.data[i].d_port_min
      r.d_port_max = res.data[i].d_port_max
      r.proto = (res.data[i].protocol == 6) ? "Tcp" : (res.data[i].protocol == 17) ? "Udp" : "Icmp"
      r.act = (res.data[i].action == 1) ? "接收" : "丢弃"
      r.log = (res.data[i].logging == 1) ? "是" : "否"
      r.timeout = res.data[i].timeout
      r.protocol = res.data[i].protocol
      r.action = res.data[i].action
      r.logging = res.data[i].logging
      ruleTableData.value.push(r)
    }
  })
  getDefAct().then(res => {
    defaultAction.value.action = res.data.default_act == 1 ? "接收" : "丢弃"
    defaultAction.value.default_act = res.data.default_act
  })
}



export default {
  name: 'RuleManagement',
  data: function(){
    return {
      drawer,
      oneRule,
      rules,
      drawerAction,
      defaultAction,
      ruleTableData
    }
  },
  methods: {
    submitRule() {
      this.$refs.ruleFormRef.validate((valid) => {
        if(!valid) {
          this.$message.warning('请调整数据后再请求')
          return false
        }
        // operation code
        if (this.oneRule.protocol === 1)
          this.oneRule.s_port_min = this.oneRule.s_port_max = this.oneRule.d_port_min = this.oneRule.d_port_max = 1
        if (this.drawerAction === '新建规则') {
          this.oneRule.id = 0
          addRule(this.oneRule).then(res => {
            console.log(res)
            this.$message.success('添加规则成功')
            this.drawer = false
            refresh()
          }).catch(err => {
            console.log(err)
            this.$message.error(err + ' 添加规则失败')
          })
        }
        else {
          // update rule
          modifyRule(this.oneRule).then(res => {
            console.log(res)
            this.$message.success('修改规则成功')
            this.drawer = false
            refresh()
          }).catch(err => {
            console.log(err)
            this.$message.error(err + ' 修改规则失败')
          })
        }
      })
    },
    reFresh() {
      refresh()
    },
    setDefAct() {
      defaultAction.value.default_act = 1 - defaultAction.value.default_act
      defaultAction.value.action = defaultAction.value.default_act == 1 ? "接收" : "丢弃"
      setDefAct(defaultAction.value).then(res => {
        console.log(res)
        this.$message.success('设置默认动作成功')
      }).catch(err => {
        console.log(err)
        this.$message.error(err + ' 设置默认动作失败')
      })
    },
    delRule(id) {
      var data = {
        id: id
      }
      delRule(data).then(res => {
        console.log(res)
        this.$message.success('删除规则成功')
        refresh()
      }).catch(err => {
        console.log(err)
        this.$message.error(err + ' 删除规则失败')
      })
    }
  },
  setup() {
    onMounted(() => {
      refresh()
    })
  }
  
}
</script>