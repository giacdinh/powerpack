#!/bin/bash
openssl x509 -text < /mnt/sysdata/certs/unit-cert.pem |grep Subject:
