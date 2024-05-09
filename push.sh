set -euxo pipefail
CR_ID=$( tofu output --state=tf/terraform.tfstate --raw cr)
docker build -f Dockerfile --push -t cr.yandex/$CR_ID/bf:v13 /tmp/build