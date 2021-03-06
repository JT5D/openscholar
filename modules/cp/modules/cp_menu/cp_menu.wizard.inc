<?php

/**
 * @file cp_menu.wizard.inc
 *
 * Include file for cp_menu link maker forms and for helper functions that only assist the wizard
 **/


/**
 * Multi-step form wizard for adding links to menu.
 * @param unknown_type $js
 * @param unknown_type $step
 */
function cp_menu_page($js = NULL, $step = NULL, $menu = NULL) {
  //can we collect the original menu?  should that be part of the menu link so the result can be added back in?

  //get cached results
  $object_id = 'cp_menu';
  if (empty($step) || $step == 'new') {
    $step = 'type';
    cp_menu_cache_clear($object_id);
  }
  $object = cp_menu_cache_get($object_id);
  $object['js'] = ($js != 'nojs');

  //check if step is a menu?  if so, stash it and clear it.
  ctools_include('menu','os');
  if (in_array($menu, array_flip(os_get_menus()))) {
    $object['menu'] = $menu;
    cp_menu_cache_set($object_id, $object);
  }

  //forge a form state
  $form_state = array(
    'ajax' => $js,
    'object_id' => $object_id,
    'object' => &$object,
  );

  //prep multistep form info

  $form_info = array(
    'id' => 'cp_menu_form',
    'path' => 'cp/menu/' . ($js ? 'ajax' : 'nojs') . '/%step',
    'show trail' => FALSE,
    'show back' => TRUE,
    'show cancel' => TRUE,
    'next callback' => 'cp_menu_wizard_next',
    'finish callback' => 'cp_menu_wizard_finish',
    'cancel callback' => 'cp_menu_wizard_cancel',
    'forms' => array(
      'type' => array(
      	'form id' => 'cp_menu_type',
        'title'   => t('Link Type'),
      ),
    ),
  );

  //add type dependent pages based on type form's results
  if (isset($_POST['type'])) {
    $type = $_POST['type'];
    $object['type'] = $type;
    cp_menu_cache_set($object_id, $object);
  } elseif (isset($object['type'])) {
    $type = $object['type'];
  } else {
    $type = NULL;
  }

  //new node gets continue instead of finish because node creation is another step, albeit outside of wizard
  if (isset($object['type']) && $object['type'] == 'page') {
    $form_info['finish text'] = t('Continue');
  }


  if ($type == 'list') {
    $form_info['forms']['list_type'] = array('form id' => 'cp_menu_list_type', 'title'=>t('Type of List'));

    $list_type = (isset($object['list_type'])) ? $object['list_type'] : $_POST['list_type'];
    if ($list_type == 'taxonomy') {
      $form_info['forms']['term'] = array('form id' => 'cp_menu_list_term', 'title'=>t('Taxonomy Terms'));
    } elseif ($list_type == 'publications') {
      $form_info; //pubs.
    }
  }

  //add final page after type sensitive pages have been included
  $form_info['forms']['title'] = array('form id' => 'cp_menu_title', 'title'=>t('Link Title'));

  //build and render the form
  ctools_include('wizard');
  $form = ctools_wizard_multistep_form($form_info, $step, $form_state);
  $output = drupal_render($form);
  if ($js) {
  	ctools_include('ajax');
  	ctools_include('modal');
  	ctools_modal_render(t('Add menu item'), $output);
  } else {
  	return $output;
  }
}


/**
 * @function cp_menu_type
 *
 * Multistep form page for collecting the type of a link where type is page, url, or list.
 */
function cp_menu_type($form, &$formstate) {
  $form_state['title'] = t('Link Type');

  $form['type'] = array(
    '#type' => 'radios',
    '#options' => array('page'=>'Page', 'url'=>'URL'),//, 'list'=>'List'),
    '#description' => t('Type of content to link to.'),
  	'#required' => TRUE,
  );

  return $form;
}

/**
 * @function cp_menu_title
 *
 * Multistep form page for setting the title and advanced options of a link.
 */
function cp_menu_title($form, &$form_state) {
  $form_state['title'] = t('Title');

  //can this be prepopulated if we're coming from an existing node?
  $form['title'] = array(
    '#title' => t('Title'),
    '#type' => 'textfield',
    '#description' => t('Name your link.'),
  	'#required' => TRUE,
  );

  //add type dependent fields
  switch($form_state['object']['type']) {
    case 'url':
      $form['url'] = array(
        '#type' => 'textfield',
        '#title' => t('URL'),
        '#required' => TRUE,
      );

      $form['#validate'][] = 'cp_menu_url_validate';
      break;

    case 'page':
      $form['new_or_select'] = array(
        '#type' => 'radios',
        '#title' => t('Destination'),
        '#options' => array('new'=>t('New Page'), 'select'=>t('Choose Existing')),
        '#description' => t('Choose a page you have already created or make a new page as the destination for your link'),
        '#required' => TRUE,
      );

      $form['node'] = array(
        '#type' => 'textfield',
        '#title' => t('Page'),
        '#autocomplete_path' => 'ctools/autocomplete/node', //does not limit by vsite or user.  waiting for d7's version of vsite_node_autocomplete
        '#description' => t('Select an existing page.'),
        '#states' => array(
          'visible' => array(
            ':input[name="new_or_select"]' => array('value' => 'select'),
          ),
        ),
      );

      $form['new_node_type'] = array(
        '#type' => 'select',
        '#title' => t('Create new'),
        '#description' => t('Create a new page.'),
        '#options' => os_get_bundles(),
        '#states' => array(
          'visible' => array(
            ':input[name="new_or_select"]' => array('value' => 'new'),
          ),
        ),
      );

      break;
  }


  //should we support other attributes here as well?  still undecided if new window should exist
  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced Options'),
    '#collapsed' => TRUE,
  	'#collapsible' => TRUE,

    'new_window' => array(
      '#type' => 'checkbox',
      '#title' => t('New Window'),
      '#description' => t('Open link in new tab or window'),
      '#default' => FALSE,
    ),

    'tooltip' => array(
      '#type' => 'textfield',
      '#title' => t('Tooltip'),
      '#description' => t('Text displayed when mouse hovers over link'),
    )
  );

  return $form;
}


/**
 * @function cp_menu_url_validate
 *
 * Validation for cp_menu_url.  Checks that URl exists.
 */
function cp_menu_url_validate(&$form, &$state) {
  if ($state['values']['url'] == '<front>') {
    return;
  }

  $url = check_url($state['values']['url']);
  $menu = menu_get_item($url);

  if (url_is_external($url)) {
    $header = @get_headers($url);

    if (!$header) {
      form_set_error('url', t('Could not connect to url: %url', array('%url'=>'$url')));
    } else {
      $http = explode(' ', $header[0]);
      $http_code = $http[1];

      if ($http_code >= 400) {
        form_set_error('url', t('URL Returned Error: %error', array('%error' => $header[0])));
      }
    }

  } elseif (!$menu) {
      form_set_error('url', t('Path does not exist'));
  }
}

/**
 * @function cp_menu_list_type
 *
 * Multistep form page for gathering further information about the type of list to link to.
 */
function cp_menu_list_type($form, &$formstate) {
  $form_state['title'] = t('Type of List');

  $form['list_type'] = array(
    '#title' => t('Type of List'),
    '#type' => 'radios',
    '#options' => array('publications'=>'Publications', 'taxonomy'=>'Taxonomy'),
    '#description' => t('Type of content to link to.'),
    '#required' => TRUE,
  );

  $form['publications_type'] = array(
    '#type' => 'radios',
    '#title' => 'Publications by ...',
    '#options' => array('year'=>'Year', 'type'=>'Type', 'co'=>'Co-author'),
    '#states' => array(
      'visible' => array(
        ':input[name="list_type"]' => array('value' => 'publications'),
      ),
    )
  );

  //is there a vsite aware version of this?
  $options = array();
  foreach (taxonomy_get_vocabularies() as $vid => $vocab) {
    $options[$vocab->vid] = $vocab->name;
  }

  if (count($options)) {
    $form['vocabulary'] = array(
      '#type' => 'radios',
      '#title' => 'Vocabulary',
      '#options' => $options,
      '#states' => array(
        'visible' => array(
          ':input[name="list_type"]' => array('value' => 'taxonomy'),
        ),
      )
    );
  } else {
    $form['taxonomy_not_found'] = array(
      '#type' => 'item',
      '#title' => 'No vocabularies found.  Please create a vocabulary or choose another type of list.',
      '#states' => array(
        'visible' => array(
          ':input[name="list_type"]' => array('value' => 'taxonomy'),
        ),
      )
    );
  }

  return $form;
}

/**
 * @function cp_menu_list_type_validate
 *
 * Validation for cp_menu_list_type.  Publication lists need a type (year, co-auth, etc) and taxonomy lists need a vocabulary.
 * FAPI #states weren't quite up to the task of imposing conditional requirements.
 */
function cp_menu_list_type_validate(&$form, &$formstate) {
  if ($formstate['values']['list_type'] == 'publications' && strlen($formstate['values']['publications_type']) == 0) {
    form_set_error('publications_type', t('Publication list selected, but missing publication type.'));
  }
  if ($formstate['values']['list_type'] == 'taxonomy' && strlen($formstate['values']['vocabulary']) == 0) {
    form_set_error('vocabulary', t('Taxonomy list selected, but missing vocabulary.'));
  }
}

/**
 * @function cp_menu_list_term
 *
 * Multistep form page for choosing the term to display in a vocabulary list.
 */
function cp_menu_list_term($form, &$formstate) {
  $vid = $formstate['object']['vocabulary'];
  $terms = taxonomy_get_tree($vid, 0);

  $options = array();
  foreach ($terms as $term) {
    $options[$term->tid] = $term->name;
  }

  $form['term'] = array(
    '#title' => t('Term'),
    '#type' => 'radios',
    '#options' => $options,
  );

  return $form;
}


/**
* CP menu cache storage
*/
function cp_menu_cache_set($id, $object) {
  ctools_include('object-cache');
  ctools_object_cache_set('cp_menu', $id, $object);
}

/**
* CP menu cache retrieval
*/
function cp_menu_cache_get($id) {
  ctools_include('object-cache');
  $object = ctools_object_cache_get('cp_menu', $id);
  return ($object) ? $object : array();
}

/**
* Clear the wizard cache.
*/
function cp_menu_cache_clear($id) {
  ctools_include('object-cache');
  ctools_object_cache_clear('cp_menu', $id);
}




/**
* Advance the cp menu wizard to the next page.  Concatenate cached and submitted values
*/
function cp_menu_wizard_next(&$form_state) {
  $form_state['object'] += $form_state['values'];
  cp_menu_cache_set($form_state['object_id'], $form_state['object']);
}

/**
* CP menu wizard cancelled.
*/
function cp_menu_wizard_cancel(&$form_state) {
	$cached = cp_menu_cache_get($form_state['object_id']);
  cp_menu_cache_clear($form_state['object_id']);

  if ($cached['js'] || $form_state['ajax']) {
  	ctools_include('ajax');
  	ctools_include('modal');
  	$commands = array();
  	$commands[] = ctools_modal_command_dismiss();
  	print ajax_render($commands);
  	exit;

  } else {
  	drupal_goto('<front>');
  }
}

/**
* CP menu wizard has completed.  Add results to a menu.
*/
function cp_menu_wizard_finish(&$form_state) {
  $values = cp_menu_cache_get($form_state['object_id']) + $form_state['values'];

  //new node page
  if ($values['type'] == 'page' && $values['new_or_select'] == 'new') {
    cp_menu_cache_set($form_state['object_id'], $values);

    if ($form_state['ajax']) {
    	ctools_include('ajax');
    	ctools_include('modal');
    	$commands = array();
    	$commands[] = ctools_ajax_command_redirect('node/add/' . $values['new_node_type']);
    	print ajax_render($commands);
    	exit;
    } else {
    	drupal_goto('node/add/' . $values['new_node_type']);
    }
  }

  //Build the link from the collected data
  switch ($values['type']) {
    case 'url':
      $url = check_url($values['url']);
      break;

    case 'page':
      $nid = (int)array_pop(explode(' ', $values['node']));
      $url = 'node/'.$nid;
      break;

    case 'list':
      break;

    default:
      return;
  }

  $menu_name = isset($values['menu']) ? $values['menu'] : 'primary-menu';
  $options = array();

  if ($values['new_window']) {
    $new_menu['options']['attributes']['target'] = '_blank';
  }

  if ($values['tooltip']) {
    $options['description'] = $values['tooltip'];
  }

  //then save to vsite or whole site
  ctools_include('menu','os');
  $item = array('link_path' => $url, 'menu_name' => $menu_name, 'link_title' => $values['title'],'options' => $options);
  $mlid = os_menu_link_save($item);
  cp_menu_cache_clear($form_state['object_id']);
  if ($form_state['ajax']) {
  	ctools_include('ajax');
  	ctools_include('modal');
  	$commands = array();
  	$commands[] = ctools_modal_command_dismiss();
  	$commands[] = ctools_ajax_command_reload();
  	print ajax_render($commands);
  	exit;

  } else {
  	drupal_goto('<front>');
  }
}

