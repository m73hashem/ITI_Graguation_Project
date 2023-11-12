from . import views
from django.urls import path

urlpatterns = [
    #path('', views.index, name='index'),
    path('', views.home, name='home'),
    path('download/', views.download, name='download'),
    path('create', views.create, name='create'),
    path('list', views.list, name='list'),
    path('fileupload', views.fileupload, name='fileupload'),
    path('edit/<int:id>', views.edit, name='edit'),  # Update this line
    path('edit/update/<int:id>', views.update, name='update'),  # Update this line
    path('delete/<int:id>', views.delete, name='delete'),  # Update this line
    path('register/', views.register, name='register'),
    path('register/success/', views.register_success, name='register_success'),
    path('users/', views.users, name='users'),
    path('users/delete/<int:id>', views.user_delete, name='user_delete'),  # Update this line
    path('change_password', views.changePassword, name='changePassword'),
    path('file/delete', views.changePassword, name='changePassword'),  # This line seems incorrect
    path('file/delete/<int:id>', views.deleteFiles, name='deleteFiles'),  # Update this line
]