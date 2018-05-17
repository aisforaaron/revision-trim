## Revision Trim Module


### Goal

Remove outdated node revisions to improve site performance. 


### Install

1. Enable the module and dependencies 
2. Update the settings page /admin/config/content/revision-trim
3. Enable file_lock module (if needed)
4. Configure Elysia Cron to run revision_trim_delete_revisions at some interval
5. Wait for cron to run or use admin settings form to force run


### Files

Drupal core behavior is on revision delete to remove any files that don't have any more
associations to nodes/paragraphs. If you want to keep these files, use the file_lock
module with the below settings.

File Lock module
- /admin/config/media/lock
  - lock files by pattern
  - pattern: *
  - hooks: all file_save actions (insert and update)
