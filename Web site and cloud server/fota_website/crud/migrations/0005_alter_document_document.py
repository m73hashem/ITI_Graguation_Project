# Generated by Django 4.2.6 on 2023-11-04 17:46

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ("crud", "0004_document_title"),
    ]

    operations = [
        migrations.AlterField(
            model_name="document",
            name="document",
            field=models.FileField(upload_to="media/documents/"),
        ),
    ]