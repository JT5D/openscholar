<?php
/**
 * @file
 * vsite.features.user_permission.inc
 */

/**
 * Implements hook_user_default_permissions().
 */
function vsite_user_default_permissions() {
  $permissions = array();

  // Exported permission: administer group.
  $permissions['administer group'] = array(
    'name' => 'administer group',
    'roles' => array(
      0 => 'administrator',
    ),
    'module' => 'og',
  );

  // Exported permission: create vsite content.
  $permissions['create vsite content'] = array(
    'name' => 'create vsite content',
    'roles' => array(
      0 => 'authenticated user',
    ),
    'module' => 'node',
  );

  // Exported permission: create vsite content on behalf.
  $permissions['create vsite content on behalf'] = array(
    'name' => 'create vsite content on behalf',
    'roles' => array(
      0 => 'administrator',
      1 => 'manager',
    ),
    'module' => 'vsite',
  );

  // Exported permission: delete any vsite content.
  $permissions['delete any vsite content'] = array(
    'name' => 'delete any vsite content',
    'roles' => array(
      0 => 'administrator',
      1 => 'manager',
    ),
    'module' => 'node',
  );

  // Exported permission: delete own vsite content.
  $permissions['delete own vsite content'] = array(
    'name' => 'delete own vsite content',
    'roles' => array(
      0 => 'authenticated user',
    ),
    'module' => 'node',
  );

  // Exported permission: edit any vsite content.
  $permissions['edit any vsite content'] = array(
    'name' => 'edit any vsite content',
    'roles' => array(
      0 => 'administrator',
      1 => 'manager',
    ),
    'module' => 'node',
  );

  // Exported permission: edit own vsite content.
  $permissions['edit own vsite content'] = array(
    'name' => 'edit own vsite content',
    'roles' => array(
      0 => 'authenticated user',
    ),
    'module' => 'node',
  );

  // Exported permission: manage group space.
  $permissions['manage group space'] = array(
    'name' => 'manage group space',
    'roles' => array(
      0 => 'administrator',
      1 => 'manager',
      2 => 'vsite admin',
    ),
    'module' => 'spaces_og',
  );

  // Exported permission: view users outside groups.
  $permissions['view users outside groups'] = array(
    'name' => 'view users outside groups',
    'roles' => array(
      0 => 'authenticated user',
    ),
    'module' => 'spaces_og',
  );

  return $permissions;
}
