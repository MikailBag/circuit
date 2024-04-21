resource "yandex_container_registry" "main" {
    name = "circuit"
    labels = {
        "managed-by" = "tf"
    }
}

resource "yandex_container_repository" "bf" {
    name = "${yandex_container_registry.main.id}/bf"
}

resource "yandex_container_repository_lifecycle_policy" "name" {
  name = "ttl"
  repository_id = yandex_container_repository.bf.id
  status = "active"

  rule {
    expire_period = "168h"
    untagged = true
    retained_top = 1
  }
}

module "prepare-ig" {
    source = "../../mycloud/lib/instancegroups-shared"
    folder_id = var.folder_id
}

data "yandex_compute_image" "ubuntu" {
  family = "container-optimized-image"
}

resource "yandex_vpc_network" "main" {
    labels = {
        "managed-by" = "tf"
    }
}

resource "yandex_vpc_subnet" "main" {
    network_id = yandex_vpc_network.main.id
    labels = {
        "managed-by" = "tf"
    }
    zone = "ru-central1-a"
    v4_cidr_blocks = ["10.0.100.0/24"]
}

resource "yandex_iam_service_account" "vm" {
    name = "vm"
    description = "[TF]"
}

resource "yandex_container_repository_iam_binding" "pull" {
    repository_id = yandex_container_repository.bf.id
    role = "container-registry.images.puller"
    members = ["serviceAccount:${yandex_iam_service_account.vm.id}"]
}

resource "yandex_compute_instance_group" "main" {
    service_account_id = module.prepare-ig.service_account_id
    labels = {
        "managed-by" = "tf"
    }
    name = "my"
    instance_template {
        service_account_id = yandex_iam_service_account.vm.id
        resources {
            cores = 4
            memory = 4
        }
        network_interface {
            subnet_ids = [yandex_vpc_subnet.main.id]
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
            "docker-compose" = file("${path.module}/spec.yaml")
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
