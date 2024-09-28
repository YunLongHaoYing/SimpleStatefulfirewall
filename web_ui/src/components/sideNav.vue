<template>
    <div class="aside" style="height: 100%; width: 100%;">
      <el-row class="tac" style="height: 100%; width: 100%;">
        <el-col :span="20" style="height: 100%; width: 100%;">
          <el-menu 
            default-active="this.$route.path" router
            class="el-menu-vertical-demo"
            @open="handleOpen"
            @close="handleClose"
            style="height: 100%; width: 120%;">
            <el-menu-item index="/RuleManagement">
                <el-icon><Operation /></el-icon>
                <span>规则管理</span>
            </el-menu-item>
            <el-menu-item index="/PackageLog">
                <el-icon><Switch /></el-icon>
                <span>流量记录</span>
            </el-menu-item>
            <el-menu-item index="/OperationLog">
                <el-icon><Clock /></el-icon>
                <span>操作记录</span>
            </el-menu-item>
          </el-menu>
        </el-col>
      </el-row>
    </div>
</template>
  
<script>

export default {
    methods: {
        handleOpen(key, keyPath) {
          console.log(key, keyPath);
        },
        handleClose(key, keyPath) {
          console.log(key, keyPath);
        }
    }
}
</script>
  