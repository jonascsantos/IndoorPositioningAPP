/**
 * Define all of your application routes here
 * for more information on routes, see the
 * official documentation https://router.vuejs.org/en/
 */
export default [
  {
    path: '/dashboard',
    // Relative to /src/views
    view: 'Dashboard',
    name: 'Panel'
  },
  {
    path: '/user-profile',
    name: 'User Profile',
    view: 'UserProfile'
  },
  {
    path: '/table-list/:sensorId',
    name: 'Historical Data',
    view: 'TableList',
    props: true
  },
  {
    path: '/typography',
    view: 'Typography'
  },
  {
    path: '/icons',
    view: 'Icons'
  },
  {
    path: '/maps/:sensorId?',
    name: 'Mapa',
    view: 'Maps',
    props: true
  },
  {
    path: '/notifications',
    view: 'Notifications'
  },
  {
    path: '/upgrade',
    name: 'Upgrade to PRO',
    view: 'Upgrade'
  }
]
