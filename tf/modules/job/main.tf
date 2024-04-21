data "yandex_compute_image" "ubuntu" {
  family = "container-optimized-image"
}

resource "yandex_compute_instance_group" "main" {
    service_account_id = var.ig_service_account_id
    labels = {
        "managed-by" = "tf"
    }
    name = "job-${var.name}"
    instance_template {
        service_account_id = var.vm_service_account_id
        resources {
            cores = 4
            memory = 4
        }
        network_interface {
            subnet_ids = [var.subnet_id]
            nat = true
        }
        boot_disk {
            initialize_params {
                size = "15"
                type = "network-hdd"
                image_id = data.yandex_compute_image.ubuntu.id
            }
        }
        metadata = {
            "enable-oslogin" = "true"
            "docker-compose" = var.spec
        }
        scheduling_policy {
            preemptible = false
        }
        metadata_options {
            aws_v1_http_endpoint = 0
            aws_v1_http_token    = 0
            gce_http_endpoint    = 1
            gce_http_token       = 1
        }

    }
    scale_policy {
        fixed_scale {
            size = 1
        }
    }
    deploy_policy {
        max_expansion = 1
        max_unavailable = 1
    }
    allocation_policy {
        zones = ["ru-central1-a"]
    }
}