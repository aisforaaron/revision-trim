<?php

/**
 * Admin settings form.
 */
function revision_trim_settings_form() {

  // Run Trim on demand and redirect back to form.
  if (isset($_GET['trim']) && $_GET['trim'] == 'yes') {
    revision_trim_delete_revisions();
    drupal_goto('/admin/config/content/revision-trim');
  }

  // Get vars

  $trimTypes = variable_get('revision_trim_types', null);
  $trimCount = variable_get('revision_trim_count', 100);
  $trimAge   = variable_get('revision_trim_age', 0);
  $trimNid   = variable_get('revision_trim_nid', 0);

  // Get one node per query
  // don't run query unless we have good values.
  if($trimTypes !== null && $trimAge !== null) {
    $count = db_query("select n.nid
    from {node} n
    left join {node_revision} nr ON n.nid = nr.nid
    where n.type IN (:types)
    AND nr.timestamp < UNIX_TIMESTAMP(DATE_SUB(NOW(), INTERVAL :age DAY))
    group by n.nid
    having count(nr.nid) > :amount",
      [
        ':types' => $trimTypes,
        ':amount' => $trimCount + 1,
        ':age' => $trimAge,
      ])->rowCount();
  }
  $form = array();
  $form['messages'] = array(
    '#type' => 'markup',
    '#markup' => t('Amount of nodes of certain types with over your max count of revisions. This module will check each revision against the updated timestamp before deleting.<br><br>Count: :count <br><br><a href="?trim=yes">Trim a node >></a><br><br><hr><br>', [':count' => $count]),
  );
  $form['revision_trim_count'] = array(
    '#type' => 'textfield',
    '#title' => t('Amount of revisions to keep'),
    '#size' => 3,
    '#maxlength' => 3,
    '#default_value' => $trimCount,
    '#required' => TRUE,
  );
  // Get timestamp of x-amount of time (like -90 days) from today.
  if(!empty($trimAge)) {
    $dateLine = date_create();
    date_sub($dateLine, date_interval_create_from_date_string($trimAge. ' days'));
    $trimAgeMsg = t('Revisions before: ').date_format($dateLine, 'm/d/Y');
  }
  else {
    $trimAgeMsg = t('Leave as zero to get revisions from today.');
  }
  $form['revision_trim_age'] = array(
    '#type' => 'textfield',
    '#title' => t('Trim revisions that are at least this many days old'),
    '#description' => $trimAgeMsg,
    '#size' => 3,
    '#maxlength' => 3,
    '#default_value' => $trimAge,
    '#required' => TRUE,
  );
  $form['revision_trim_nid'] = array(
    '#type' => 'textfield',
    '#title' => t('Trim revisions on a specific node id'),
    '#description' => t('Leave empty or as zero to bypass this and get next node from database.'),
    '#size' => 10,
    '#maxlength' => 10,
    '#default_value' => $trimNid,
    '#required' => TRUE,
  );
  $form['revision_trim_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Select content types to trim'),
    '#description' => t('Only look at these content types when trimming revisions.'),
    '#options' => node_type_get_names(),
    '#default_value' => $trimTypes,
    '#required' => TRUE,
  );

  return system_settings_form($form);
}
