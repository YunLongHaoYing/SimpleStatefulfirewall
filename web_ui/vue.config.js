const { defineConfig } = require('@vue/cli-service')
module.exports = defineConfig({
  transpileDependencies: true,
  devServer: {
    proxy: {
      '/myfw': {
        target: 'http://localhost:8888',
        changeOrigin: true
      }
    }
  }
})


