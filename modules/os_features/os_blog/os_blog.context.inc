<?php
// $Id$


/**
 * @file
 * os_blog.context.inc
 */

/**
 * Implements hook_context_default_contexts().
 */
function os_blog_context_default_contexts() {
  $export = array();

  $context = new stdClass();
  /* Edit this to true to make a default context disabled initially */
  $context->disabled = FALSE;
  $context->api_version = 3;
  $context->name = 'blog_blog';
  $context->description = 'Archive of recent blog entries';
  $context->tag = 'OS features';
  $context->conditions = array(
    'node' => array(
      'values' => array(
        'blog' => 'blog',
      ),
      'options' => array(
        'node_form' => '0',
      ),
    ),
    'path' => array(
      'values' => array(
        'blog' => 'blog',
        'blog/*' => 'blog/*',
      ),
    ),
    'views' => array(
      'values' => array(
        'os_blog:page' => 'os_blog:page',
      ),
    ),
  );
  $context->reactions = array(
    'block' => array(
      'blocks' => array(
        'boxes-os_taxonomy_fltr_cat' => array(
          'module' => 'boxes',
          'delta' => 'os_taxonomy_fltr_cat',
          'region' => 'sidebar_second',
          'weight' => '-10',
        ),
        'boxes-os_addthis' => array(
          'module' => 'boxes',
          'delta' => 'os_addthis',
          'region' => 'sidebar_second',
          'weight' => '-9',
        ),
        'views-os_blog-block' => array(
          'module' => 'views',
          'delta' => 'os_blog-block',
          'region' => 'sidebar_second',
          'weight' => '-8',
        ),
        'views-os_blog_comments-block' => array(
          'module' => 'views',
          'delta' => 'os_blog_comments-block',
          'region' => 'sidebar_second',
          'weight' => '-7',
        ),
        'boxes-blog_rss_feed' => array(
          'module' => 'boxes',
          'delta' => 'blog_rss_feed',
          'region' => 'footer',
          'weight' => '-10',
        ),
      ),
    ),
    'menu' => 'blog',
  );
  $context->condition_mode = 0;

  // Translatables
  // Included for use with string extractors like potx.
  t('Archive of recent blog entries');
  t('OS features');
  $export['blog_blog'] = $context;

  return $export;
}

