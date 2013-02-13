<?php

// User Migration class
class UserMigration extends Migration {
  // class constructor
  public function __construct() {
    parent::__construct(MigrateGroup::getInstance('examplegroup'));
    $this->description = t('Migrate Drupal 6 users');
 
    // Source
    $source_fields = array(
      'uid' => t('User ID'),
      'roles' => t('The set of roles assigned to a user.'),
    );
    
    // import active users only from the old db
    $connection = commerce_migrate_ubercart_get_source_connection();
    $query = $connection->select('users', 'u')
    //$query = db_select(OUR_SOURCE_DATABASE .'.users', 'u')
      ->fields('u', array('uid', 'name', 'pass', 'mail', 'created', 'access', 'login', 'status', 'init'))
      ->condition('u.status', 1, '=')
      ->condition('u.uid', 1, '>')
      ->orderBy('u.uid', 'ASC');
    $this->source = new MigrateSourceSQL($query, $source_fields);

    // Destination
    $this->destination = new MigrateDestinationUser(array('md5_passwords' => TRUE));
 
    // Mappings
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'uid' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'D6 Unique User ID',
          'alias' => 'u',
        )
      ),
      MigrateDestinationUser::getKeySchema()
    );
 
    // Create the field mappings
    $this->addFieldMapping('is_new')->defaultValue(TRUE);
    $this->addFieldMapping('uid', 'uid');
    $this->addFieldMapping('name', 'name')->dedupe('users', 'name');
    $this->addFieldMapping('pass', 'pass');
    $this->addFieldMapping('mail', 'mail')->dedupe('users', 'mail');
    $this->addFieldMapping('language')->defaultValue('');
    $this->addFieldMapping('theme')->defaultValue('');
    $this->addFieldMapping('signature')->defaultValue('');
    $this->addFieldMapping('signature_format')->defaultValue('filtered_html');
    $this->addFieldMapping('created', 'created');
    $this->addFieldMapping('access', 'access');
    $this->addFieldMapping('login', 'login');
    $this->addFieldMapping('status', 'status');
    $this->addFieldMapping('picture')->defaultValue(0);
    $this->addFieldMapping('init', 'init');
    $this->addFieldMapping('timezone')->defaultValue(NULL);
    $this->addFieldMapping('path')->issueGroup(t('DNM')); // DNM = Do Not Map
    $this->addFieldMapping('roles', 'roles');
  }
  
  // massage the user roles before saving
  public function prepareRow($current_row) {
    $source_id = $current_row->uid;
    //$query = db_select(OUR_SOURCE_DATABASE .'.users_roles', 'r')
    $connection = commerce_migrate_ubercart_get_source_connection();
    $query = $connection->select('users_roles', 'r')
      ->fields('r', array('uid', 'rid'))
      ->condition('r.uid', $source_id, '=');
    $results = $query->execute();
    // add the Authenticated role for everyone
    $roles = array('2' => '2');
    foreach ($results as $row) {
      // Editors (rid 3) need role id 4
      if ($row->rid == 3) {
        $roles['4'] = '4';
      }
      // Admins (rid 4) need role id 3
      if ($row->rid == 4) {
        $roles['3'] = '3';
      }
      // Collaboraters are role id 5 in both old and new dbs
      $roles[$row->rid] = $row->rid;
    }
    $current_row->roles = $roles;
    return TRUE;
    // return FALSE if you wish to skip a particular row
  }
} // close the ourUserMigration class
