<template>
    <div>
        <el-text class="mx-1" size="large">
            <el-icon><Clock /></el-icon>
            操作记录
        </el-text>
        <el-table :data="operationTableData" style="width: 100%">
            <el-table-column prop="time" label="时间" width="240"></el-table-column>
            <el-table-column prop="operation" label="操作" width="300"></el-table-column>
            <el-table-column prop="res" label="结果" width="135"></el-table-column>
            <el-table-column prop="msg" label="错误信息" width="300"></el-table-column>
        </el-table>
        <el-pagination
            background
            :pager-count="11"
            layout="prev, pager, next"
            :page-count="PageSetting.pageNum"
            v-model:current-page="PageSetting.page"
            @current-change="readMLog(PageSetting.page)"
        />
    </div>
</template>

<script>
import { onMounted, ref } from 'vue'
import { readMLog} from '@/api/log';

const PageSetting = ref({
    page: 1,
    pageNum: 1
})

const operationTableData = ref([
])

function OpF() {
    this.time = "";
    this.operation = "";
    this.res = "";
    this.msg = "";
}

function reflesh() {
    readMLog(1).then(res => {
        operationTableData.value = [];
        if( res.data.data){
            res.data.data.forEach(element => {
                let op = new OpF();
                op.time = element.created_at;
                op.operation = element.operation;
                op.res = element.result === 1 ? "成功" : "失败";
                op.msg = element.error_msg;
                operationTableData.value.push(op);
            });
        }
        PageSetting.value.pageNum = Math.ceil(res.data.total / 10);
    });
}

export default {
  name: 'OperationLog',
  data(){
    return {
        operationTableData,
        PageSetting
    }
  },
  methods: {
    readMLog(page) {
        readMLog(page).then(res => {
            operationTableData.value = [];
            if( res.data.data){
                res.data.data.forEach(element => {
                    let op = new OpF();
                    op.time = element.created_at;
                    op.operation = element.operation;
                    op.res = element.result === 1 ? "成功" : "失败";
                    op.msg = element.error_msg;
                    operationTableData.value.push(op);
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