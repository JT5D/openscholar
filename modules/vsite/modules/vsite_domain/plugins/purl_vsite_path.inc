<?php
/**
 * Create the processor that will override the purl_path enables path + domain
 */
if (!class_exists('purl_vsite_path')) {

  /**
   *  Full path + domain handling for scholar
   */
  class purl_vsite_path extends purl_path implements purl_processor {

    public static $a_disabled = array();
    //allowing domain suffix to have alphanumeric chars, hyphens, underscores, minimum of 3 chararcters long
    public $modifier_regex = '!^[a-z0-9_-]{3,}+$!';

    public function method() {
      return PURL_VSITE_PATH;
    }

    /**
     * Make sure the path is valid
     */
    public function parse($valid_values, $q) {
      $valid_values = array_diff_key($valid_values, _vsite_domain_processed_purls());
      $a_parsed = parent::parse($valid_values, $q);

      foreach ($a_parsed as $s_site => $a_settings) {
        if ($a_settings->provider == 'spaces_og' && variable_get('purl_base_domain', FALSE)) {
        	$vsite_domain = db_query("SELECT value FROM {spaces_overrides} WHERE type = :type AND id = :id AND object_id = 'vsite_domain_name' AND object_type = 'variable' ", array(':type' => 'og', ':id' => $a_settings->id, ))->fetchField();
          if($vsite_domain) $vsite_domain = unserialize($vsite_domain);

          if(!is_string($vsite_domain)) $vsite_domain = false;

          if (in_array($a_settings->value, self::$a_disabled) ||
             (strlen($vsite_domain) && str_replace('http://', '', $_SERVER['HTTP_HOST']) != $vsite_domain)
 //            || (!strlen($vsite_domain) && str_replace('http://', '', $_SERVER['HTTP_HOST']) != str_replace('http://', '', variable_get('purl_base_domain', 'scholar.iq.harvard.edu')))
             ) {
            unset($a_parsed[$s_site]);
          }
        }
      }//Do not return space's without the correct domain
      return $a_parsed;
    }

	  public function rewrite(&$path, &$options, $element) {

	  	$plugin = ctools_get_plugins('spaces', 'plugins','vsite');
      if($plugin) $class = ctools_plugin_get_class($plugin, 'handler');
      //Do not alter excluded paths
      if(isset($class) && call_user_func(array($class,'is_excluded_path'),$path)) return;

	    if(array_key_exists('absolute',$options) && $options['absolute']){

	      if($element->id && $vsite = spaces_load('og',$element->id)){
          $vsite_domain = $vsite->controllers->variable->get('vsite_domain_name');
        }

        if($vsite_domain){
          $options['base_url'] = "http://".$vsite_domain;
        }else{
        	global $base_url;
          $options['base_url'] = variable_get('purl_base_domain', $base_url);
        }//Check for a group domain

      }//If we are asking for the absolute url then return the correct base url for a path provider

      //Disable addition of URL prefix if prefix is part of the path alias
      if (!empty($path) && $path != '<front>') {
        $language = isset($options['language']) && isset($options['language']->language) ? $options['language']->language : '';
        $alias = drupal_get_path_alias($path, $language);
        if ($alias != $path && $alias != $this->remove($alias, $element)) {
          return;
        }
      }

	    return parent::rewrite($path, $options, $element);
	  }
  }

}
