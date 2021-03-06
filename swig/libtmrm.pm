# This file was automatically generated by SWIG (http://www.swig.org).
# Version 1.3.38
#
# Do not make changes to this file unless you know what you are doing--modify
# the SWIG interface file instead.

package libtmrm;
use base qw(Exporter);
use base qw(DynaLoader);
package libtmrmc;
bootstrap libtmrm;
package libtmrm;
@EXPORT = qw();

# ---------- BASE METHODS -------------

package libtmrm;

sub TIEHASH {
    my ($classname,$obj) = @_;
    return bless $obj, $classname;
}

sub CLEAR { }

sub FIRSTKEY { }

sub NEXTKEY { }

sub FETCH {
    my ($self,$field) = @_;
    my $member_func = "swig_${field}_get";
    $self->$member_func();
}

sub STORE {
    my ($self,$field,$newval) = @_;
    my $member_func = "swig_${field}_set";
    $self->$member_func($newval);
}

sub this {
    my $ptr = shift;
    return tied(%$ptr);
}


# ------- FUNCTION WRAPPERS --------

package libtmrm;

*tmrm_object_get_type = *libtmrmc::tmrm_object_get_type;
*tmrm_subject_map_sphere_new = *libtmrmc::tmrm_subject_map_sphere_new;
*tmrm_subject_map_sphere_free = *libtmrmc::tmrm_subject_map_sphere_free;
*tmrm_subject_map_new = *libtmrmc::tmrm_subject_map_new;
*tmrm_subject_map_iterator = *libtmrmc::tmrm_subject_map_iterator;
*tmrm_subject_map_proxies = *libtmrmc::tmrm_subject_map_proxies;
*tmrm_subject_map_merge = *libtmrmc::tmrm_subject_map_merge;
*tmrm_subject_map_export_to_yaml = *libtmrmc::tmrm_subject_map_export_to_yaml;
*tmrm_subject_map_is_value_by_key = *libtmrmc::tmrm_subject_map_is_value_by_key;
*tmrm_subject_map_name = *libtmrmc::tmrm_subject_map_name;
*tmrm_subject_map_bottom = *libtmrmc::tmrm_subject_map_bottom;
*tmrm_subject_map_free = *libtmrmc::tmrm_subject_map_free;
*tmrm_proxy_new = *libtmrmc::tmrm_proxy_new;
*tmrm_proxy_by_label = *libtmrmc::tmrm_proxy_by_label;
*tmrm_proxy_clone = *libtmrmc::tmrm_proxy_clone;
*tmrm_proxy_update = *libtmrmc::tmrm_proxy_update;
*tmrm_proxy_label = *libtmrmc::tmrm_proxy_label;
*tmrm_proxy_keys = *libtmrmc::tmrm_proxy_keys;
*tmrm_proxy_keys_by_value = *libtmrmc::tmrm_proxy_keys_by_value;
*tmrm_proxy_values_by_key = *libtmrmc::tmrm_proxy_values_by_key;
*tmrm_proxy_values_by_key_t = *libtmrmc::tmrm_proxy_values_by_key_t;
*tmrm_proxy_is_value_by_key = *libtmrmc::tmrm_proxy_is_value_by_key;
*tmrm_proxy_is_referenced = *libtmrmc::tmrm_proxy_is_referenced;
*tmrm_proxy_remove = *libtmrmc::tmrm_proxy_remove;
*tmrm_proxy_equals = *libtmrmc::tmrm_proxy_equals;
*tmrm_proxy_add_property = *libtmrmc::tmrm_proxy_add_property;
*tmrm_proxy_add_property_literal = *libtmrmc::tmrm_proxy_add_property_literal;
*tmrm_proxy_remove_properties_by_key = *libtmrmc::tmrm_proxy_remove_properties_by_key;
*tmrm_proxy_get_properties = *libtmrmc::tmrm_proxy_get_properties;
*tmrm_proxy_add_type = *libtmrmc::tmrm_proxy_add_type;
*tmrm_proxy_add_superclass = *libtmrmc::tmrm_proxy_add_superclass;
*tmrm_proxy_subclasses = *libtmrmc::tmrm_proxy_subclasses;
*tmrm_proxy_superclasses = *libtmrmc::tmrm_proxy_superclasses;
*tmrm_proxy_direct_types = *libtmrmc::tmrm_proxy_direct_types;
*tmrm_proxy_direct_instances = *libtmrmc::tmrm_proxy_direct_instances;
*tmrm_proxy_direct_subclasses = *libtmrmc::tmrm_proxy_direct_subclasses;
*tmrm_proxy_direct_superclasses = *libtmrmc::tmrm_proxy_direct_superclasses;
*tmrm_proxy_sub = *libtmrmc::tmrm_proxy_sub;
*tmrm_proxy_isa = *libtmrmc::tmrm_proxy_isa;
*tmrm_proxy_free = *libtmrmc::tmrm_proxy_free;
*tmrm_literal_new = *libtmrmc::tmrm_literal_new;
*tmrm_literal_datatype = *libtmrmc::tmrm_literal_datatype;
*tmrm_literal_value = *libtmrmc::tmrm_literal_value;
*tmrm_literal_is_value_by_key = *libtmrmc::tmrm_literal_is_value_by_key;
*tmrm_literal_keys_by_value = *libtmrmc::tmrm_literal_keys_by_value;
*tmrm_literal_free = *libtmrmc::tmrm_literal_free;
*tmrm_multiset_new = *libtmrmc::tmrm_multiset_new;
*tmrm_multiset_new_from_iterator = *libtmrmc::tmrm_multiset_new_from_iterator;
*tmrm_multiset_insert = *libtmrmc::tmrm_multiset_insert;
*tmrm_multiset_add = *libtmrmc::tmrm_multiset_add;
*tmrm_multiset_size = *libtmrmc::tmrm_multiset_size;
*tmrm_multiset_as_list = *libtmrmc::tmrm_multiset_as_list;
*tmrm_multiset_free = *libtmrmc::tmrm_multiset_free;
*tmrm_iterator_new = *libtmrmc::tmrm_iterator_new;
*tmrm_iterator_next = *libtmrmc::tmrm_iterator_next;
*tmrm_iterator_end = *libtmrmc::tmrm_iterator_end;
*tmrm_iterator_get_object = *libtmrmc::tmrm_iterator_get_object;
*tmrm_iterator_get_key = *libtmrmc::tmrm_iterator_get_key;
*tmrm_iterator_get_value = *libtmrmc::tmrm_iterator_get_value;
*tmrm_iterator_free = *libtmrmc::tmrm_iterator_free;

# ------- VARIABLE STUBS --------

package libtmrm;

*TMRM_ITERATOR_GET_METHOD_GET_OBJECT = *libtmrmc::TMRM_ITERATOR_GET_METHOD_GET_OBJECT;
*TMRM_ITERATOR_GET_METHOD_GET_CONTEXT = *libtmrmc::TMRM_ITERATOR_GET_METHOD_GET_CONTEXT;
*TMRM_ITERATOR_GET_METHOD_GET_KEY = *libtmrmc::TMRM_ITERATOR_GET_METHOD_GET_KEY;
*TMRM_ITERATOR_GET_METHOD_GET_VALUE = *libtmrmc::TMRM_ITERATOR_GET_METHOD_GET_VALUE;
*TMRM_TYPE_UNKNOWN = *libtmrmc::TMRM_TYPE_UNKNOWN;
*TMRM_TYPE_PROXY = *libtmrmc::TMRM_TYPE_PROXY;
*TMRM_TYPE_LITERAL = *libtmrmc::TMRM_TYPE_LITERAL;
*TMRM_TYPE_MULTISET = *libtmrmc::TMRM_TYPE_MULTISET;
*TMRM_TYPE_ITERATOR = *libtmrmc::TMRM_TYPE_ITERATOR;
*TMRM_TYPE_TUPLE = *libtmrmc::TMRM_TYPE_TUPLE;
*TMRM_TYPE_SEQUENCE = *libtmrmc::TMRM_TYPE_SEQUENCE;
1;
