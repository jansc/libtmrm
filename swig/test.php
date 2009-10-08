<?php
include('libtmrm.php'); 

$sms = tmrm_subject_map_sphere_new();
$storage = tmrm_storage_new($sms, "pgsql", "host='localhost',dbname='tmrm_test',user='jans'");
$m = tmrm_subject_map_new($sms, $storage, "mymap");

$ontology =<<<EOS
%YAML 1.1
---
subject_map: 'base'
proxies:
    bottom: {libtmrm_bottom: libtmrm_bottom}
    item_identifier: {bottom: "item-identifier"}
    subject_identifier: {bottom: "subject-identifier"}
    subject_locator: {bottom: "subject-locator"}
    member: {bottom: "member"}
    type: {bottom: "type"}
    subject: {bottom: "subject"}
    scope: {bottom: "scope"}
    reified: {bottom: "reified"}
    reifier: {bottom: "reifier"}
...
EOS;

//$fh = fopen("/Users/jans/labben/libtmrm/tests/yaml/t2.yaml", "rw");
//tmrm_subject_map_import_from_yaml($m, $fh);

tmrm_subject_map_import_from_yaml_string($m, $ontology);

$p = tmrm_proxy_new($m);
$key = tmrm_proxy_new($m);
$value = tmrm_proxy_new($m);
tmrm_proxy_add_property($p, $key, $value);

$literal = tmrm_literal_new("Terje", "http://www.w3.org/2001/XMLSchema#string");
tmrm_proxy_add_property_literal($p, $key, $literal);
tmrm_literal_free($literal);

tmrm_proxy_free($key);
tmrm_proxy_free($value);
tmrm_proxy_free($p);

//tmrm_subject_map_export_to_yaml

$proxies = tmrm_subject_map_iterator($m);

while (!tmrm_iterator_end($proxies)) {
    $obj = tmrm_iterator_get_object($proxies);
    if (tmrm_object_get_type($obj) == TMRM_TYPE_PROXY) {
        $proxy = tmrm_object_to_proxy($obj);
        printf("Found proxy with label '%s'\n", tmrm_proxy_label($proxy));
        tmrm_proxy_free($proxy);
    }
    tmrm_iterator_next($proxies);
}


tmrm_subject_map_free($m);
tmrm_storage_free($storage);
tmrm_subject_map_sphere_free($sms);

?>
