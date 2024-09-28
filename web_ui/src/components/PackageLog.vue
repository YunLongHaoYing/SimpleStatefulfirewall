<template>
    <div>
        <el-text class="mx-1" size="large">
            <el-icon><Connection /></el-icon>
            连接
        </el-text>
        <el-table :data="connetionTableData"  style="width: 100%">
            <el-table-column prop="saddr" label="源IP" width="135"></el-table-column>
            <el-table-column prop="daddr" label="目的IP" width="135"></el-table-column>
            <el-table-column prop="proto" label="协议" width="120"></el-table-column>
            <el-table-column prop="sport" label="源端口" width="120"></el-table-column>
            <el-table-column prop="dport" label="目的端口" width="120"></el-table-column>
            <el-table-column prop="state" label="状态" width="60"></el-table-column>
            <el-table-column prop="type" label="Type" width="80"></el-table-column>
            <el-table-column prop="code" label="Code" width="80"></el-table-column>
            <el-table-column prop="timeout" label="过期时间" width="240"></el-table-column>
        </el-table>
    </div>
    <el-divider />
    <div>
        <el-text class="mx-1" size="large">
            <el-icon><Switch /></el-icon>
            流量
        </el-text>
        <el-table :data="packgaeTableData" style="width: 100%">
            <el-table-column prop="time" label="时间" width="240"></el-table-column>
            <el-table-column prop="saddr" label="源IP" width="135"></el-table-column>
            <el-table-column prop="daddr" label="目的IP" width="135"></el-table-column>
            <el-table-column prop="proto" label="协议" width="120"></el-table-column>
            <el-table-column prop="sport" label="源端口" width="120"></el-table-column>
            <el-table-column prop="dport" label="目的端口" width="120"></el-table-column>
            <el-table-column prop="state" label="状态" width="60"></el-table-column>
            <el-table-column prop="type" label="Type" width="80"></el-table-column>
            <el-table-column prop="code" label="Code" width="80"></el-table-column>
            <el-table-column prop="len" label="长度" width="120"></el-table-column>
            <el-table-column prop="act" label="操作" width="60"></el-table-column>
        </el-table>
        <el-pagination
            background
            :pager-count="11"
            layout="prev, pager, next"
            :page-count="PageSetting.pageNum"
            v-model:current-page="PageSetting.page"
            @current-change="readPLog(PageSetting.page)"
        />
    </div>
</template>

<script>
import { onMounted, ref } from 'vue'
import { getConnetionList, readPLog} from '@/api/log';
const connetionTableData = ref([
])

const PageSetting = ref({
    page: 1,
    pageNum: 1
})

function ConnF() {
    this.saddr = "";
    this.daddr = "";
    this.proto = "";
    this.sport = "";
    this.dport = "";
    this.state = "";
    this.type = "";
    this.code = "";
    this.timeout = "";
}

const packgaeTableData = ref([
])

function PackF() {
    this.time = "";
    this.saddr = "";
    this.daddr = "";
    this.proto = "";
    this.sport = "";
    this.dport = "";
    this.state = "";
    this.type = "";
    this.code = "";
    this.len = "";
    this.act = "";
}

function reflesh() {
    getConnetionList().then(res => {
        connetionTableData.value = [];
        if (res.data.connections) {
            res.data.connections.forEach(element => {
                let conn = new ConnF();
                conn.saddr = element.saddr;
                conn.daddr = element.daddr;
                conn.proto = element.protocol == 6 ? "TCP" : element.protocol == 17 ? "UDP" : "ICMP";
                conn.sport = element.sport + "";
                conn.dport = element.dport + "";
                if (element.protocol === 1)
                    conn.sport = conn.dport = "-";
                conn.state = element.state;
                if (element.protocol !== 6)
                    conn.state = "-";
                conn.type = element.type;
                conn.code = element.code;
                if (element.protocol !== 1)
                    conn.code = "-";
                conn.timeout = element.timeout;
                connetionTableData.value.push(conn);
            });
        }
        
    });
    readPLog(1).then(res => {
        packgaeTableData.value = [];
        if( res.data.data){
            res.data.data.forEach(element => {
                let pack = new PackF();
                pack.time = element.time;
                pack.saddr = element.saddr;
                pack.daddr = element.daddr;
                pack.proto = element.protocol == 6 ? "TCP" : element.protocol == 17 ? "UDP" : "ICMP";
                pack.sport = element.sport + "";
                pack.dport = element.dport + "";
                if (element.protocol === 1)
                    pack.sport = pack.dport = "-";
                pack.state = element.state;
                if (element.protocol !== 6)
                    pack.state = "-";
                pack.type = element.type;
                pack.code = element.code;
                if (element.protocol !== 1)
                    pack.code = "-";
                pack.len = element.len;
                pack.act = element.action === 1 ? "接收" : "丢弃";
                packgaeTableData.value.push(pack);
            });
        }
        PageSetting.value.pageNum = Math.ceil(res.data.total / 10);
    });
}

export default {
  name: 'HelloWorld',
  data(){
    return {
        connetionTableData,
        packgaeTableData,
        PageSetting
    }
  },
  methods: {
    readPLog(page) {
        readPLog(page).then(res => {
            packgaeTableData.value = [];
            if (res.data.data){
                res.data.data.forEach(element => {
                    let pack = new PackF();
                    pack.time = element.time;
                    pack.saddr = element.saddr;
                    pack.daddr = element.daddr;
                    pack.proto = element.protocol == 6 ? "TCP" : element.protocol == 17 ? "UDP" : "ICMP";
                    pack.sport = element.sport + "";
                    pack.dport = element.dport + "";
                    if (element.protocol === 1)
                        pack.sport = pack.dport = "-";
                    pack.state = element.state;
                    if (element.protocol !== 6)
                        pack.state = "-";
                    pack.type = element.type;
                    pack.code = element.code;
                    if (element.protocol !== 1)
                        pack.code = "-";
                    pack.len = element.len;
                    pack.act = element.action === 1 ? "接收" : "丢弃";
                    packgaeTableData.value.push(pack);
                });
            }
            PageSetting.value.pageNum = Math.ceil(res.data.total / 10);
        });
    }
  },
  setup() {
    onMounted(() => {
        reflesh();
    });
  }
}
</script>