//引入依赖及组件
import { createRouter } from "vue-router"
import { createWebHistory } from "vue-router"

import pagesOne from '../components/HelloWorld'
import RuleManagement from '@/components/RuleManagement'
import PackageLog from '@/components/PackageLog'
import OperationLog from '@/components/OperationLog'

const routes = [
  {
    path: '/',
    name: 'HelloWorld',
    component: pagesOne
  },
  {
    path: '/RuleManagement',
    name: 'RuleManagement',
    component: RuleManagement
  },
  {
    path: '/PackageLog',
    name: 'PackageLog',
    component: PackageLog
  },
  {
    path: '/OperationLog',
    name: 'OperationLog',
    component: OperationLog
  }
];

const router = createRouter({ 
  history: createWebHistory(),
  routes 
});

export default router
