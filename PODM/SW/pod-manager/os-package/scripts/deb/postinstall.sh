# Copyright (c) 2015-2016 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

USER_NAME="podm"

echo_log() {
    echo "[${DPKG_MAINTSCRIPT_PACKAGE}]: ${1}"
}

configure_podm_configuration_agent_service() {
    echo_log "Configuring Pod Manager Configuration Agent service"
    update-rc.d pod-manager-configuration-agent defaults 25
    service pod-manager-configuration-agent start
}

change_permissions() {
    echo_log 'Changing files permissions'

    chmod 755 /usr/bin/pod-manager-configuration-agent
    chmod 755 /usr/bin/pod-manager-networking
    chmod 755 /usr/bin/pod-manager-clean-database-on-next-startup
    chmod 755 /etc/init.d/pod-manager-configuration-agent

    mkdir -p /var/lib/pod-manager
    chown -R ${USER_NAME}:${USER_NAME} /var/lib/pod-manager
}

check_if_user_exists() {
    echo_log "Checking if user ${USER_NAME} exists"
    if ! id -u ${USER_NAME} > /dev/null 2>&1; then
        echo_log "User ${USER_NAME} does not exist. This might happen when upgrading from the old Pod Manager packages."
        echo_log "Please create user: ${USER_NAME} OR purge old package and then reinstall this one."
        exit 1
    fi
}

trigger_wildfly_virgin_startup_sequnce() {
    # disable orientdb WAL
    sed -i "s/USE_WAL=.*/USE_WAL=false/g" /etc/default/orientdb
    sudo service orientdb restart

    java -jar /usr/lib/pod-manager/db-schema-creation.jar

    # enable orientdb WAL
    sed -i "s/USE_WAL=.*/USE_WAL=true/g" /etc/default/orientdb
    service orientdb restart
    service wildfly start
}

echo_log "postinst: beginning ${1} action"
check_if_user_exists
mkdir -p /var/log/pod-manager
chown -R ${USER_NAME}:${USER_NAME} /var/log/pod-manager

# configuring keystore for WildFly's HTTPS
chown -R ${USER_NAME}:${USER_NAME} /var/lib/pod-manager
chmod 755 /usr/bin/pod-manager-certificate-creation.sh
chmod 755 /usr/bin/pod-manager-certificate-import.sh
chmod 766 /usr/lib/pod-manager/certificate-common-functions.sh
chmod 664 /usr/lib/pod-manager/vault-decrypter.jar

echo_log "configuring orientdb..."
chown -R ${USER_NAME}:${USER_NAME} /opt/pod-manager/orientdb

chmod -R 600 /opt/pod-manager/orientdb/config/*
chmod 700 /opt/pod-manager/orientdb/config
chmod 700 /opt/pod-manager/orientdb/config/cert
chmod 755 /etc/init.d/orientdb

update-rc.d orientdb defaults 20

# configuring WildFly
echo_log "configuring wildfly..."
chown -R ${USER_NAME}:${USER_NAME} /opt/pod-manager/wildfly
ln -f -s /opt/pod-manager/wildfly/bin/init.d/wildfly-init-debian.sh /etc/init.d/wildfly
ln -f -s /opt/pod-manager/wildfly/bin/init.d/wildfly.conf /etc/default/wildfly
update-rc.d wildfly defaults 30

trigger_wildfly_virgin_startup_sequnce

change_permissions
configure_podm_configuration_agent_service

echo_log "postinst: done"

