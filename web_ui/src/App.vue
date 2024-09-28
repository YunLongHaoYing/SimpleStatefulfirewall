<template>
  <div class="app">
    <el-container
      style="border: 1px solid #eee;">
      <!-- 头部 -->
      <myHeader></myHeader>
    </el-container>

    <el-container 
      style="height: 100vh; 
      border: 1px solid #eee; 
      margin: 0;
      border: 0;
      padding: 0;">     <!-- style： 隐藏滚动条  去掉白边-->
      <!-- 侧边栏 -->
      <el-aside style="width: 120px;">
        <asideNav></asideNav>
      </el-aside>
      <!-- 主页面 -->
      <el-main>
        <router-view></router-view>  
      </el-main>
    </el-container>
  </div>
</template>
 
<script>
import myHeader from "@/components/myHeader"
import asideNav from "@/components/sideNav"


 
export default {
  name: "App",
  components: {
    myHeader,
    asideNav,
  }
}
</script>

<style>
  html,
  body{
    height: 100%;
    margin: 0;
    border: 0;
    padding: 0;
  }
</style>
