#!/usr/bin/perl -w

use Gtk;         # load the Gtk-Perl module
use strict;      # a good idea for all non-trivial Perl scripts
use threads;     # Threading
use Getopt::Long;

set_locale Gtk;  # internationalize
init Gtk;        # initialize Gtk-Perl

use constant TRUE  => 1;
use constant FALSE => 0;

# options vars
use vars qw( $progname 
             $usage 
             $opt_help 
             $opt_debug );

($progname = $0) = ~s(.*/)();
$usage = <<USAGE;
Usage: $progname [options]
Options:
    -debug     Allows PANA debug message to be shown in parent shell.
               Also disables iwconfig polling.
    -help      print this message
USAGE

# entry config vars
use vars qw( $entry_username
             $entry_password
             $entry_cfgfile
             $entry_shared_secret
             $entry_auth_script
             $entry_dhcp_bootstrap
             $entry_use_archie
             $entry_eap_auth_period
             $entry_thread_count );

# label config vars
use vars qw( $entry_pana_status
             $entry_ip_installed
             $entry_ip_preauth
             $entry_pana_color
             $entry_pana_font
             $entry_pana_font_name );

$entry_pana_font_name = "lucidasans-18";

# pana threading vars
use vars qw( $pana_setup_file
             $pana_is_active );

$pana_setup_file = "config/pana_gui_setup.xml";
$pana_is_active = FALSE;

# IP change monitor polling vars
use vars qw( $iwmon_interval
             $iwmon_iwconfig
             $iwmon_args
             $iwmon_script
             $iwmon_phy_if
             $iwmon_prov_if
             $iwmon_ifconfig
             $iwmon_thd );

$iwmon_interval = 0.200; # 200 msec checking
$iwmon_iwconfig = '/usr/src/sipmm/iwconfig1';
$iwmon_args     = "";
$iwmon_script   = './test';
$iwmon_ifconfig = '/sbin/ifconfig';
$iwmon_phy_if   = "eth0";
$iwmon_prov_if  = "eth1";

# options
GetOptions(qw(debug! help!)) or die $usage;
if ($opt_help) {
    print $usage;
    exit 0;
}

# widget creation
my $window_dialer  = SetupDialerWindow( 600, 400, 15 );
my $hbox_controls  = SetupControlBox();
my $frame_config   = SetupConfigFrame( $entry_username,
                                       $entry_password,
                                       $entry_cfgfile,
                                       $entry_shared_secret,
                                       $entry_auth_script,
                                       $entry_eap_auth_period,
                                       $entry_thread_count,
                                       $entry_dhcp_bootstrap,
                                       $entry_use_archie );
my $frame_status   = SetupStatusFrame( $entry_pana_status,
                                       $entry_ip_installed,
                                       $entry_ip_preauth );
# specialization
$entry_password->set_visibility( FALSE );

# main box
my $vbox_main      = SetupMainBox($frame_config, 
                                  $frame_status,
                                  $hbox_controls );

# set text properties for pana status
SetTextProperties();

# window layout
$window_dialer->add($vbox_main);
$window_dialer->show();

# start aux threads
$iwmon_thd = threads->new( \&IpAddressChangeThread );

# Gtk event loop
main Gtk;

# Should never get here
exit( 0 );

### Generate configuration string
sub GenerateConfigXMLString
{
    my $text = "";
    my $xmlStr = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    $xmlStr .= "<configuration xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n";
    $xmlStr .= "                    xsi:noNamespaceSchemaLocation=\'pana_setup.xsd\'>\n";
    $text = $entry_cfgfile->get_chars( 0, -1 );
    $xmlStr .= "   <pana_cfg_file>$text</pana_cfg_file>\n";
    $text = $entry_username->get_chars( 0, -1 );
    $xmlStr .= "   <username>$text</username>\n";
    $text = $entry_password->get_chars( 0, -1 );
    $xmlStr .= "   <password>$text</password>\n";
    $text = $entry_shared_secret->get_chars( 0, -1 );
    $xmlStr .= "   <secret>$text</secret>\n";
    $text = $entry_auth_script->get_chars( 0, -1 );
    $xmlStr .= "   <auth_script>$text</auth_script>\n";
    if ( $entry_dhcp_bootstrap->active ) {
        $xmlStr .= "   <dhcp_bootstrap>1</dhcp_bootstrap>\n";
    }
    else {
        $xmlStr .= "   <dhcp_bootstrap>0</dhcp_bootstrap>\n";
    }
    if ( $entry_use_archie->active ) {
        $xmlStr .= "   <use_archie>1</use_archie>\n";
    }
    else {
        $xmlStr .= "   <use_archie>0</use_archie>\n";
    }
    $text = $entry_eap_auth_period->get_chars( 0, -1 );
    $xmlStr .= "   <auth_period>$text</auth_period>\n";
    $text = $entry_thread_count->get_chars( 0, -1 );
    $xmlStr .= "   <thread_count>$text</thread_count>\n";
    $xmlStr .= "   <update_pickup_file>config/gotip</update_pickup_file>\n";
    $xmlStr .= "</configuration>\n";
    return $xmlStr;
}

### Setup Dialer window
sub SetupDialerWindow
{
    my ( $hsize, $vsize, $bwidth ) = @_;
    my $window_dialer  = new Gtk::Window( "toplevel" );

    # window layout
    $window_dialer->set_title( "PANA Dialer" );
    $window_dialer->border_width( $bwidth );
    $window_dialer->set_usize( $hsize, $vsize );

    # callback registration
    $window_dialer->signal_connect( "delete_event", \&CloseDialer );   
    return $window_dialer;
}

### Setup Main box
sub SetupMainBox
{
    my $vbox_main = new Gtk::VBox( FALSE, 5 );
    my $count = scalar( @_ );
    for ( my $i = 0; $i < $count; $i ++ ) {
        $vbox_main->pack_start( $_[$i], TRUE, TRUE, 0 );
        $_[$i]->show();
    }
    $vbox_main->show();
    return $vbox_main;
}

### Setup Status box
sub SetupStatusFrame
{
    my $frame_status = new Gtk::Frame( "Status" );
    my $vbox_status = new Gtk::VBox( FALSE, 5 );
    my $count = scalar( @_ );
    my @ip_labels = ( "Physical IP : ",
                      "Provisioned IP : " );

    # PANA status
    my $hbox_pana_status = new Gtk::HBox( FALSE, 5 );

    my $label_pana = new Gtk::Label( "PANA Status : " );
    $label_pana->set_justify( 'left' );
    $hbox_pana_status->pack_start( $label_pana, FALSE, FALSE, 0 );
    $label_pana->show();

    # Create a table
    my $table_pana = new Gtk::Table( 2, 2, FALSE );
    $table_pana->set_row_spacing( 0, 2 );
    $table_pana->set_col_spacing( 0, 2 );
    $hbox_pana_status->pack_start( $table_pana, TRUE, TRUE, 0 );
    $table_pana->show();

    # Create the Text widget
    $_[0] = new Gtk::Text( undef, undef );
    $_[0]->set_editable( FALSE );
    $table_pana->attach( $_[0], 0, 1, 0, 1,
                        [ 'expand', 'shrink', 'fill' ],
                        [ 'expand', 'shrink', 'fill' ],
                        0, 0 );
    $_[0]->show();

    # Add a vertical scrollbar to the GtkText widget
    my $vscrollbar_pana = new Gtk::VScrollbar( $_[0]->vadj );
    $table_pana->attach( $vscrollbar_pana, 1, 2, 0, 1, 'fill',
                         [ 'expand', 'shrink', 'fill' ], 0, 0 );
    $vscrollbar_pana->show();

    $vbox_status->pack_start( $hbox_pana_status, FALSE, FALSE, 0 );
    $hbox_pana_status->show();

    # IP addresses
    my $hbox_ip_address = new Gtk::HBox( FALSE, 5 );
    for ( my $i = 0; $i < scalar(@ip_labels); $i ++ ) {

        my $label_ip = new Gtk::Label( $ip_labels[$i] );
        $label_ip->set_justify( 'left' );
        $hbox_ip_address->pack_start( $label_ip, FALSE, FALSE, 0 );
        $label_ip->show();

        $_[$i+1] = new Gtk::Entry( 120 );
        $_[$i+1]->set_editable( FALSE );
        $hbox_ip_address->pack_start( $_[$i+1], TRUE, TRUE, 0 );
        $_[$i+1]->show();
    }

    $vbox_status->pack_start( $hbox_ip_address, FALSE, FALSE, 0 );
    $hbox_ip_address->show();

    $frame_status->add( $vbox_status );
    $vbox_status->show();
    $frame_status->show();
    return $frame_status;
}

#### Set PANA status font and color
sub SetTextProperties
{
    # Get the system color map and allocate the color red
    my $cmap_pana = Gtk::Gdk::Colormap->get_system();
    $entry_pana_color->{ 'red' } = 0xFFFF;
    $entry_pana_color->{ 'green' } = 0;
    $entry_pana_color->{ 'blue' } = 0;

    unless ( defined( $cmap_pana->color_alloc( $entry_pana_color ) ) ) {
       warn( "Couldn't allocate color\n" );
    }

    # Load a fixed font
    $entry_pana_font = Gtk::Gdk::Font->load( $entry_pana_font_name );
}

#### Set current PANA status
sub SetTextPanaStatus
{
    my $line = "$_[0]\n";   
    $entry_pana_status->insert( $entry_pana_font, 
                                $entry_pana_color,
                                undef, $line );
}

### Setup Configuration frame
sub SetupConfigFrame
{
    my $frame_cfg = new Gtk::Frame( "Configuration" );
    my $vbox_cfg = new Gtk::VBox( FALSE, 5 );
    my @texts = ( "user2",
                  "",
                  "config/pana_pac_unix.xml",
                  "config/shared_secret.bin",
                  "config/setup-tunnel",
                  "900",
                  "5",
                  "check",
                  "check" );
    my @labels = ( " User :",
                   " Password :",
                   " Configuration File :",
                   " Shared Secret File : ",
                   " Auth Script :",
                   " EAP Auth Period :",
                   " Thread Count :",
                   " DHCP Boostrap",
                   " Use Archie" );
    my $count = scalar( @_ );
    my $max = scalar( @texts );
    for ( my $i = 0; $i < $count && $i < $max; $i ++ ) {

        my $hbox_entry = new Gtk::HBox( FALSE, 5 );

        if ( $texts[$i] eq "check" ) {
            $_[$i] = new Gtk::CheckButton( $labels[$i] );
            $_[$i]->set_active( TRUE );
            $hbox_entry->pack_start( $_[$i], FALSE, FALSE, 0 );
            $_[$i]->show();
        }
        else {
            my $label = new Gtk::Label( $labels[$i] );
            $label->set_justify( 'left' );
            $hbox_entry->pack_start( $label, FALSE, FALSE, 0 );
            $label->show();

            $_[$i] = new Gtk::Entry( 120 );
            $_[$i]->set_text( $texts[$i] );
            $hbox_entry->pack_start( $_[$i], TRUE, TRUE, 0 );
            $_[$i]->show();
	}

        $vbox_cfg->pack_start( $hbox_entry, TRUE, TRUE, 0 );
        $hbox_entry->show();
    }
    $vbox_cfg->show();
    $frame_cfg->add( $vbox_cfg );
    $frame_cfg->show();
    return $frame_cfg;
}

### Setup control box
sub SetupControlBox
{
    my $hbox_control = new Gtk::HBox( FALSE, 5 );
    my @texts = ( "Connect",
                  "Cancel" );
    my @functions = ( \&StartPana,
                      \&StopPana );
    my $count = scalar( @texts );
    for ( my $i = 0; $i < $count; $i ++ ) {
        my $control = new Gtk::Button( $texts[$i] );
        $control->signal_connect( "clicked", $functions[$i] );
        $hbox_control->pack_start( $control, TRUE, TRUE, 0 );
        $control->show();
    }
    return $hbox_control;
}

### Callback function to start pana
sub StartPana
{
    if ( $pana_is_active == FALSE ) {
        $pana_is_active = TRUE;
        my $thrd = threads->new( \&PanaThread );
   }
}

### Callback function to stop pana
sub StopPana
{    
    my $pana_kill = "kill -9 \`ps -ax | grep pacd | awk \'{print \$1}\'\`";
    system($pana_kill) == -1
	and warn "$0: $pana_kill exec failed: $!\n";
    $pana_is_active = FALSE;
    SetTextPanaStatus( "Pana session stopped" );
}

### PANA Thread
sub PanaThread
{
    open( SETUP, ">$pana_setup_file" );
    my $xmlStr = GenerateConfigXMLString();
    print SETUP $xmlStr;
    close( SETUP );

    my $result = FALSE;
    open ( PANASTAT, "./pacd -f $pana_setup_file 2>&1 |" );
    while ( <PANASTAT> ){
	 if ( $opt_debug ) {
 	     print $_;
	 }
	 if ( /TxPDI/ ) {
             SetTextPanaStatus( "Discovering PAA ...." );
	 }
         elsif ( /TxPAN/ ) {
             SetTextPanaStatus( "Receiving messages, client Authentication in progress ..." );
	 }
         elsif ( /Authentication success/ ) {
             SetTextPanaStatus( "User Authenticated ..." );
             $result = TRUE;
	 }
         elsif ( /Authentication failure/ ) {
             SetTextPanaStatus( "Authentication failed" );
             $result = FALSE;
	 }
         elsif ( /TxPUR/ ) {
             SetTextPanaStatus( "Sending IP update request" );
	 }
         elsif ( /TxPUA/ ) {
             SetTextPanaStatus( "IP update request accepted" );
	 }
         elsif ( /Disconnect: cause/ ) {
             if ( $result == FALSE ) {
                 SetTextPanaStatus( "Session disconnected, authentication failed" );
	    }
	    else {
                 SetTextPanaStatus( "Session disconnected succesfully" );
	    }
	 }
    }
    close( PANASTAT );
    $pana_is_active = FALSE;
}

### Callback function to check for change of address
sub IpAddressChangeThread
{    
    my ( $oldch, $newch ) = ( 0, 0 );
    ($oldch) = (`$iwmon_iwconfig $iwmon_args` =~ /Channel:(\d+)/);
    for (;;) {
        select(undef, undef, undef, $iwmon_interval);
        if ( ! $opt_debug ) {
            ($newch) = (`$iwmon_iwconfig $iwmon_args` =~ /Channel:(\d+)/);
            system($iwmon_script, $oldch, $newch) == -1
	         and warn "$0: $iwmon_script exec failed: $!\n" if $oldch != $newch;
#            print $newch;
            $oldch = $newch;
	}

        my @entries = ( $entry_ip_installed,
                        $entry_ip_preauth );
        my @ifaces = ( $iwmon_phy_if, 
                       $iwmon_prov_if );
        for ( my $i = 0; $i < 2; $i ++ ) {
            my $display = "";
            if ( open( NEWIP, "$iwmon_ifconfig $ifaces[$i] |" ) ) {
		my $line = <NEWIP>;
                if ( <NEWIP> =~ /addr:([0-9]+.[0-9]+.[0-9]+.[0-9]+)/ ) {
                    $display = $1;
                }
                close( NEWIP );
            }
            $entries[$i]->set_text( $display );
        }
    }
}

### Callback function to close the window
sub CloseDialer
{
   Gtk->exit( 0 );
   return FALSE;
}

# END EXAMPLE PROGRAM
