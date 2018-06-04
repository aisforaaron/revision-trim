<?php

// @todo add a way to report on nodes with large revisions that aren't published.

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
  $trimSkipNids = variable_get('revision_trim_skip_nids', 0);
  $count     = 0;
  $displayNids = [];

  // Convert csv value of nids from string to array.
  $trimSkip = !empty($trimSkipNids) ? explode(', ', trim($trimSkipNids)) : [0];

  // Get one node per query
  // don't run query unless we have good values.
  if ($trimNid) {
    $displayNids = $trimNid;
    $count = '1 nid set in this form.';
  }
  else if($trimTypes !== null && $trimAge !== null) {
    $results = db_query("select n.nid
    from {node} n
    left join {node_revision} nr ON n.nid = nr.nid
    where n.type IN (:types)
    AND n.status <> 0
    AND n.nid NOT IN (:skipNids)
    AND nr.timestamp < UNIX_TIMESTAMP(DATE_SUB(NOW(), INTERVAL :age DAY))
    group by n.nid
    having count(nr.nid) > :amount
    order by count(nr.nid) DESC",
      [
        ':types' => $trimTypes,
        ':amount' => $trimCount + 1,
        ':age' => $trimAge,
        ':skipNids' => $trimSkip
      ]);
    // Get nids to display
    foreach ($results as $result) {
      $nids[] = $result->nid;
    }
    $count = count($nids) . ' nodes to trim';
    // Chop if more than 100 nids to display
    if($count > 100) {
      $displayNids = implode(', ', array_slice($nids, 0, 100)) . '...(more than 100 found)';
    }
    else {
      $displayNids = implode(', ', $nids);
    }
  }

  // Settings form.

  $form = array();
  $form['messages'] = array(
    '#type' => 'markup',
    '#markup' => t('Notes <li>This module will check the amount of nodes that meet the conditions set below and delete revisions.</li><li>This module will check each revision against the updated timestamp before deleting.</li><li>Nodes without a published revision will be skipped.</li><li>Entities are not trimmed due to how entity translations use revisions.</li><li>Watchdog messages are created with the trim results after each run.</li><br>Count: :count <br><br>Nodes to trim in order: :nids<br><br><a href="?trim=yes" onclick="return confirm(\'Are you sure? This deletes revisions for the first (or only) nid listed.\')">Trim the first node from above >></a>', [':count' => $count, ':nids' => $displayNids]),
  );


  // Trim by settings

  $form['trim_by'] = array(
    '#type' => 'fieldset',
    '#title' => 'Trim by settings',
    '#description' => t('You can think of these two settings as "Keep ____ number of revisions before date ____."'),
  );
  $form['trim_by']['revision_trim_count'] = array(
    '#type' => 'textfield',
    '#title' => t('Amount of revisions to keep'),
    '#size' => 3,
    '#maxlength' => 3,
    '#default_value' => $trimCount,
    '#description' => t('This count starts counting at the "days old" value below.<br> Example: <br>- 40 revisions newer than the trim date<br> - 100 revisions older than the trim date<br>- keep 20 revisions<br>Result will trim 80 (not 120) since it starts counting 20 before the trim date.'),
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
  $form['trim_by']['revision_trim_age'] = array(
    '#type' => 'textfield',
    // @todo swap out type for date pop and update processing dates
    //'#type' => 'date_popup',
    //'#date_format' => 'Y-m-d',
    '#title' => t('Trim revisions that are at least this many days old'),
    '#description' => $trimAgeMsg,
    '#size' => 10,
    '#maxlength' => 10,
    '#default_value' => $trimAge,
    '#required' => TRUE,
  );


  // Overrides

  $form['trim_overrides'] = array(
    '#type' => 'fieldset',
    '#title' => 'Trim overrides',
    '#description' => t('You can think of these two settings as "Keep ____ number of revisions before date ____."'),
  );
  $form['trim_overrides']['revision_trim_nid'] = array(
    '#type' => 'textfield',
    '#title' => t('Trim revisions on a specific node id'),
    '#description' => t('Leave as zero to bypass this and get next node from database.'),
    '#size' => 10,
    '#maxlength' => 10,
    '#default_value' => $trimNid,
    '#required' => TRUE,
  );
  $form['trim_overrides']['revision_trim_skip_nids'] = array(
    '#type' => 'textfield',
    '#title' => t('Skip specific node ids in trim'),
    '#description' => t('Leave as zero to bypass. If you have unpublished nodes with a high revision count they could be stuck here. Just add the nids as a csv here to skip.'),
    '#size' => 10,
    '#maxlength' => 10,
    '#default_value' => $trimSkipNids,
    '#required' => TRUE,
  );


  // Content Types

  $form['trim_types'] = array(
    '#type' => 'fieldset',
    '#title' => 'Trim types',
    '#description' => t('Only look at these content types when trimming revisions.'),
  );
  $form['trim_types']['revision_trim_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Select content types to trim'),
    '#options' => node_type_get_names(),
    '#default_value' => $trimTypes,
    '#required' => TRUE,
  );

  return system_settings_form($form);
}
