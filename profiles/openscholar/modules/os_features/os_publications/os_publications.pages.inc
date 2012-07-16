<?php

function os_publications_overview() {
  $args = func_get_args();

  if (count($args) == 0) {
    // get default types the user has selected
    $defaults = variable_get('os_publications_default_types', array());
    if (count($defaults)) {
      $args = array('type', $defaults);
    }
  }

  module_load_include('inc', 'biblio', 'includes/biblio.pages');

  return call_user_func_array('biblio_page', $args);
}

function os_publications_export($format = "tagged", $nid = NULL, $popup = FALSE) {
  module_load_include('inc', 'biblio', 'biblio.import.export');

  $title = 'publications';
  if ($nid !== null) {
    $node = node_load($nid);
    $title = str_replace(' ', '_', $node->title);
    $title = substr($title, 0, 8);
  }
  elseif (empty($_SESSION['last_biblio_query'])) {
    // Biblio uses this session value, if this is an anon user that has no session (or a cli user) lets set this up for them
    $args = array();
    // get default types the user has selected
    $defaults = variable_get('os_publications_default_types', array());
    if (count($defaults)) {
      $args = array('type', $defaults);
    }
    // Setup the default query
    module_load_include('inc', 'biblio', 'biblio.pages');
    biblio_build_query($args);
  }

  ob_start();
  $output = biblio_export($format, $nid, $popup);
  if (empty($output)) {
    // the export didn't return anything, which means it output directly to the browser
    $output = ob_get_clean();

    // handle for spaces
    if (module_exists('spaces') && $space = spaces_get_space()) {
      $title = $space->group->name.'-'.$title;
    }

    $owner_name = $vsite->group->name;
    switch ($format) {
      case 'tagged':
        drupal_add_http_header('Content-Disposition', 'filename="'.$title.'-tagged.enw"');
      	break;
      case 'xml':
      case 'endnote8':
      	drupal_add_http_header('Content-Disposition', 'attachment; filename="'.$title.'.xml"');
      break;
      case 'bibtex':
      	drupal_add_http_header('Content-Disposition', 'filename="'.$title.'-Bibtex.bib"');
      break;
      case 'csv':
        drupal_add_http_header('Content-Disposition', 'attachment; filename="'.$title.'-export.csv"');
      break;
    }
    echo $output;
  }
  else {
    ob_end_clean();
    return $output;
  }
}