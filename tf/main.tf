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

resource "yandex_iam_service_account_static_access_key" "vm" {
  service_account_id = yandex_iam_service_account.vm.id
  description        = "static access key for object storage"
}

resource "yandex_lockbox_secret" "vm-aws" {
  name = "aws-access"
  labels = {
    "managed-by" = "tf"
  }
}

resource "yandex_lockbox_secret_version" "vm-aws" {
  secret_id = yandex_lockbox_secret.vm-aws.id
  entries {
    key = "access"
    text_value = yandex_iam_service_account_static_access_key.vm.access_key
  }
  entries {
    key = "secret"
    text_value = yandex_iam_service_account_static_access_key.vm.secret_key
  }
}

resource "yandex_container_repository_iam_binding" "pull" {
    repository_id = yandex_container_repository.bf.id
    role = "container-registry.images.puller"
    members = ["serviceAccount:${yandex_iam_service_account.vm.id}"]
}

/*resource "yandex_resourcemanager_folder_iam_member" "s3-access" {
  folder_id = var.folder_id
  role      = "storage.uploader"
  member    = "serviceAccount:${yandex_iam_service_account.vm.id}"
}*/


module "run" {
    source = "./modules/job"
    ig_service_account_id = module.prepare-ig.service_account_id
    vm_service_account_id = yandex_iam_service_account.vm.id
    spec = templatefile("${path.module}/spec-match-181.yaml", {"version" = "v17"})
    subnet_id = yandex_vpc_subnet.main.id
    name = "my3"
    enabled = true
}


module "run2" {
    source = "./modules/job"
    ig_service_account_id = module.prepare-ig.service_account_id
    vm_service_account_id = yandex_iam_service_account.vm.id
    spec = templatefile("${path.module}/spec-match-181-big.yaml", {"version" = "v17"})
    subnet_id = yandex_vpc_subnet.main.id
    name = "my2"
    enabled = true
    cores = 32
    ram = 32
}
module "s3-setup" {
    source = "./modules/s3-setup"
    folder_id = "b1g36mtre1jd6jubc7go"
}

resource "yandex_storage_bucket" "uploads" {
  access_key = module.s3-setup.access_key
  secret_key = module.s3-setup.secret_key
  bucket = "mikailbag-uploads"
  anonymous_access_flags {
    read = false
    list = false
    config_read = false
  }

  grant {
    id = yandex_iam_service_account.vm.id
    type = "CanonicalUser"
    permissions = ["READ", "WRITE"]
  }
  grant {
    id = module.s3-setup.service_account_id
    type = "CanonicalUser"
    permissions = ["FULL_CONTROL"]
  }
}