<template>
    <el-header class="top">
                <div class="logo">
                
                </div>
                <span>状态检测防火墙</span>
            </el-header>
</template>

<script>
export default {
  name: 'myHeader',
    props: {
        msg: String
    }
}
</script>


<style scoped>
.el-header{

display: flex;

justify-content: space-between;

padding-left: 0;

align-items: center;

color: #000000;

font-size: 20px;

> div{

display: flex;

align-items: center;

span{
  margin-left: 15px;
}
}
 
}
</style>
