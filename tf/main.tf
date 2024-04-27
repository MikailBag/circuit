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

moved {
  from = yandex_compute_instance_group.main
  to = module.run.yandex_compute_instance_group.main
}

moved {
  from = module.run
  to = module.run[""]
}

module "run" {
    for_each = toset(["", "-honest"])
    source = "./modules/job"
    ig_service_account_id = module.prepare-ig.service_account_id
    vm_service_account_id = yandex_iam_service_account.vm.id
    spec = templatefile("${path.module}/spec${each.key}.yaml", {"version" = "v7"})
    subnet_id = yandex_vpc_subnet.main.id
    name = "my${each.key}"
}