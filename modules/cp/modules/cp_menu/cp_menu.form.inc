<?php

/**
 * Builds a form for configuring a menu or all of them
 *
 * @param string $menu - which menu we're going to operate on by default
 * Other menus will be hidden to start and displayed when the user opts to
 * move a link to another menu
 */
function cp_menu_build_form($form, $form_state, $menu = ''){
  ctools_include('ajax');
  ctools_include('modal');
  ctools_include('menu', 'os');

  $menus = os_get_menus();

  $is_active = false;
  if (isset($menus[$menu])) {
    $is_active = true;
  }
  $weight = 5;

  $form['menus'] = array(
    '#theme' => 'cp_menu_table',
    '#tree' => true
  );

  foreach ($menus as $m => $title) {
    $form['menus'][$m] = array(
      '#weight' => (($is_active && $menu == $m) ? 1 : $weight ++),
      '#hidden' => ($is_active && $menu != $m),
      'menu-name' => array(
        '#type' => 'hidden',
        '#value' => $m,
        '#attributes' => array(
          'class' => array(
            'menu-name'
          )
        )
      )
    );
    $links = _cp_menu_flatten_tree(os_menu_tree_data($m));
    foreach ($links as $link) {
      $l = $link['link'];
      $url = url($l['link_path'], array(
        'absolute' => true
      ));
      $form['menus'][$m][$l['mlid']] = array(
        'title' => array(
          '#markup' => $l['link_title'],
          '#description' => l($url, $url)
        ),
        'edit_link' => array(
          '#markup' => ctools_modal_text_button('Edit Link', 'cp/build/' . $l['mlid'] . '/edit/nojs/', '')
        ),
        'menu' => array(
          '#type' => 'select',
          '#options' => $menus,
          '#default_value' => $l['menu_name'],
          '#attributes' => array(
            'class' => array(
              'menu-name',
              'menu-name-' . $l['menu_name']
            )
          )
        ),
        'weight' => array(
          '#type' => 'textfield',
          '#default_value' => $l['weight'],
          '#size' => 3,
          '#attributes' => array(
            'class' => array(
              'menu-weight',
              'menu-weight-' . $l['menu_name']
            )
          )
        ),
        'plid' => array(
          '#type' => 'hidden',
          '#default_value' => $l['plid'],
          '#attributes' => array(
            'class' => array(
              'menu-plid',
              'menu-plid-' . $l['menu_name']
            )
          )
        ),
        'mlid' => array(
          '#type' => 'hidden',
          '#default_value' => $l['mlid'],
          '#attributes' => array(
            'class' => array(
              'menu-mlid',
              'menu-mlid' . $l['menu_name']
            )
          )
        ),
        'menu-old' => array(
          '#type' => 'hidden',
          '#value' => $l['menu_name']
        ),
        '#depth' => $l['depth']
      );
    }
  }

  $form['add_new'] = array(
    '#markup' => ctools_modal_text_button('Add New Menu', 'cp/build/menu/nojs/add', 'Add New Menu'),
  );

  $form['actions'] = array(
    '#type' => 'actions',
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Save settings')
    ),
    '#attached' => array(
      'js' => array(
        drupal_get_path('module', 'cp_menu') . '/cp_menu.form.js'
      ),
      'css' => array(
        drupal_get_path('module', 'cp_menu') . '/cp_menu.form.css'
      ),
    )
  );

  $form['#submit'] = array(
    'cp_menu_submit_form'
  );

  return $form;
}

function _cp_menu_flatten_tree($tree){
  $copy = array();
  foreach ( $tree as $l ) {
    $copy = array_merge($copy, array(
      $l
    ), _cp_menu_flatten_tree($l['below']));
  }

  return $copy;
}

/**
 * Renders the form array as a table and sets up the tabledrag
 * @param unknown_type $variables
 */
function theme_cp_menu_table($variables){
  ctools_modal_add_js();
  $table_data = $variables['table'];
  $menus = os_get_menus();

  $headers = array(
      t('Title'),
      t('Edit'),
      t('Menu'),
      t('Weight')
  );
  uasort($table_data, 'element_sort');

  $rows = array();
  foreach ( element_children($table_data) as $d ) {
    $extra_classes = array();
    if ($table_data[$d]['#hidden']) {
      $extra_classes[] = 'hidden';
    }
    $rows[] = array(
      'data' => array(
        array(
          'data' => $menus[$d] . drupal_render($table_data[$d]['menu-name']),
          'colspan' => 2,
        ),
        array(
          'data' => ctools_modal_text_button('Add New Link', 'cp/menu/nojs/new/'.$d, 'Add New Link'),
        ),
        array(
          'data' => '',
        ),
      ),
      'class' => array_merge(array(
        'section-heading'
      ), $extra_classes)
    );
    $has_links = false;
    foreach ( element_children($table_data[$d]) as $link_name ) {
      if ($link_name == 'menu-name')
        continue;
      $l = $table_data[$d][$link_name];
      $has_links = true;
      $rows[] = array(
        'data' => array(
          theme('indentation', array(
            'size' => $l['#depth'] - 1
          )) . drupal_render($l['title']),
          drupal_render($l['edit_link']),
          drupal_render($l['menu']),
          drupal_render($l['weight']) . drupal_render($l['plid']) . drupal_render($l['mlid'])
        ),
        'class' => array_merge(array(
          'draggable'
        ), $extra_classes)
      );
    }
    $rows[] = array(
      'data' => array(
        array(
          'data' => 'This menu has no links.',
          'colspan' => 4
          )
      ),
      'class' => array(
        'section-message',
        $has_links ? 'section-populated' : 'section-empty'
      )
    );
  }
  //drupal_add_tabledrag('edit-menu-table', 'match', 'sibling', 'menu-name', 'menu-name', 'menu-name', FALSE);
  if (variable_get('os_use_dropdowns', 1)) {
    drupal_add_tabledrag('edit-menu-table', 'match', 'parent', 'menu-plid', 'menu-plid', 'menu-mlid', TRUE, MENU_MAX_DEPTH - 1);
  }
  drupal_add_tabledrag('edit-menu-table', 'order', 'sibling', 'menu-weight');
  return theme('table', array(
    'header' => $headers,
    'rows' => $rows,
    'attributes' => array(
      'id' => 'edit-menu-table'
    )
  ));
}

function cp_menu_submit_form($form, &$form_state){

  //Save as normal
  foreach ($form_state['values']['menus'] as $links) {
    foreach ($links as $l) {
      if (! is_array($l))
        continue;

      $l['menu_name'] = $l['menu'];
      $link = os_menu_link_load($l['mlid'], $l['menu-old']);

      $modified = false;
      foreach (array('weight', 'menu_name', 'plid') as $param ) {
        //Has this menu links parameters been modified?
        if ($link[$param] != $l[$param]) {
          $modified = true;
          $link[$param] = $l[$param];
          //If the link is moving to a different menu, remove from the old menu
          if ($param == 'menu_name') {
            os_menu_link_delete($l['menu-old'], $l['mlid']);
          }
        }
      }

      if ($modified) {
        os_menu_link_save($link);
      }
    }
  }
}

function cp_menu_add_menu($ajax) {
  if ($ajax) {
    ctools_include('modal');
    $form_state = array(
      'ajax' => true,
    );

    $output = ctools_modal_form_wrapper('cp_menu_edit_form', $form_state);
    if ($form_state['executed']) {
      $output[] = ctools_modal_command_dismiss();
      if (isset($form_state['values']['op']) && $form_state['values']['op'] == t('Save')) {
        ctools_include('ajax');
        $output[] = ctools_ajax_command_reload();
      }
    }
    echo ajax_render($output);
    exit;
  }
  else {
    return drupal_get_form('cp_menu_edit_form');
  }
}

/**
 * Adds/Edit a menu in OpenScholar
 *
 */
function cp_menu_edit_form($form, $form_state, $type = 'add', $menu_name = false){
  ctools_include('menu', 'os');

  $title = '';

  $os_menus = os_get_menus();
  if(isset($os_menus[$menu_name])){
    $title = $os_menus[$menu_name];
  }else{
    $menu_name = false;
  }

  //Is this new or existing?
  $form['#insert'] = empty($menu_name);
  $form['old_name'] = array(
    '#type' => 'value',
    '#value' => $menu_name,
  );

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $title,
    '#required' => TRUE,
  );

  $form['menu_name'] = array(
    '#type' => 'machine_name',
    '#title' => t('Menu name'),
    '#default_value' => $menu_name,
    '#maxlength' => MENU_MAX_MENU_NAME_LENGTH_UI,
    '#description' => t('A unique name to construct the URL for the menu. It must only contain lowercase letters, numbers and hyphens.'),
    '#machine_name' => array('exists' => 'cp_menu_menu_exists'),
    // A menu's machine name cannot be changed.
    '#disabled' => !empty($menu_name),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('cp_menu_edit_submit'),
  );
  // Only custom menus may be deleted.
  $form['actions']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#access' => $type == 'edit',
    '#submit' => array('os_menu_delete_submit'),
  );

  return $form;
}

/**
 * Submit function for adding or editing a custom OpenScholar menu.
 */
function cp_menu_edit_submit($form, &$form_state) {
  $menu = $form_state['values'];

  if ($form['#insert']) {
    ctools_include('menu', 'os');

    // @todo Validation
    os_menu_add($menu['title'],$menu['menu_name']);
  }
  else {
    // @todo Edit a existing Menu's Name
  }

  drupal_set_message(t('Your menu has been saved.'));
}

/**
 * Check to see if that menu already exists
 */
function cp_menu_menu_exists($menu_name){
   ctools_include('menu', 'os');
   $menus = os_get_menus();

   return isset($menus[$menu_name]);
 }