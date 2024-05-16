variable "vm_service_account_id" {
  type = string
}

variable "ig_service_account_id" {
  type = string
}

variable "spec" {
  type = string
}

variable "subnet_id" {
  type = string
}

variable "name" {
  type = string
}

variable "enabled" {
  type = bool
  default = true
}

variable "cores" {
  type = number
  default = 16
}

variable "ram" {
  type = number
  default = 16
}