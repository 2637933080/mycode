import { createRouter, createWebHistory } from 'vue-router'
import MapView from '../components/MapView.vue'
import Login from '../components/Login.vue'
import Register from '../components/Register.vue'
import FavoritesList from '../components/FavoritesList.vue'

const routes = [
  { path: '/', name: 'home', component: MapView },
  { path: '/login', name: 'login', component: Login },
  { path: '/register', name: 'register', component: Register },
  { path: '/favorites', name: 'favorites', component: FavoritesList }
]

const router = createRouter({
  history: createWebHistory(),
  routes
})

export default router
