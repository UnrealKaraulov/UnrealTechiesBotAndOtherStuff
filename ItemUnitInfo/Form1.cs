using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Diagnostics;
using System.Windows.Forms;

namespace ItemUnitInfo
{
    public partial class Form1 : Form
    {
        public Form1 ( )
        {
            InitializeComponent( );
        }

        struct DataStr
        {
           public string ID;
           public int count;
        }


        List<string> ignoreclasslist = new List<string>( );

        void ReXfresh()
        {

            ProcessMemory war3proc = new ProcessMemory( Process.GetProcessesByName( "war3" ) [ 0 ].Id, "war3");
            war3proc.StartProcess( );
            int offset1 = war3proc.ReadInt( "Game.dll" , 0xAB4F80 );
            offset1 = war3proc.ReadInt( offset1 + 0x3BC );


            float x = war3proc.ReadFloat( offset1 + 0x310 );
            float y = war3proc.ReadFloat( offset1 + 0x314 );
            float z = war3proc.ReadFloat( offset1 + 0x318 );

            label5.Text = x.ToString( ) + "!" + y.ToString( ) + "!" + z.ToString( );

            listBox1.Items.Clear( );
            listBox2.Items.Clear( );

            int unitscount = war3proc.ReadInt( offset1 + 0x604 );
            int unitsaddr = war3proc.ReadInt( offset1 + 0x608 );

            List<DataStr> units = new List<DataStr>( );


            for ( int i = 0 ; i < unitscount ; i++ )
            {
                int curunit = i * 4 + unitsaddr;
                curunit = war3proc.ReadInt( curunit );
                byte [ ] unconvertedstr = war3proc.ReadMem( curunit + 0x30 , 4 );
                Array.Reverse( unconvertedstr );
                string curunitid = Encoding.UTF8.GetString( unconvertedstr );
                bool ignore = false;
                foreach ( string ignoreid in ignoreclasslist )
                {
                    if ( curunitid == ignoreid )
                    {
                        ignore = true;
                    }
                }

                if ( !ignore )
                {
                    DataStr tmpstr = new DataStr( );
                    tmpstr.ID = curunitid;
                    bool foundor = false;
                    for ( int n = 0 ; n < units.Count ; n++ )
                    {
                        if ( units [ n ].ID == curunitid )
                        {
                            tmpstr.count = units [ n ].count + 1;
                            units [ n ] = tmpstr;
                            foundor = true;
                        }
                    }
                    if ( !foundor )
                    {
                        tmpstr.count = 1;
                        units.Add( tmpstr );
                    }
                }
            }

            int itemscount = war3proc.ReadInt( offset1 + 0x604 + 0x10 );
            int itemsaddr = war3proc.ReadInt( offset1 + 0x608 + 0x10 );


            List<DataStr> items = new List<DataStr>( );
            for ( int i = 0 ; i < itemscount ; i++ )
            {
                int curitem = i * 4 + itemsaddr;
                curitem = war3proc.ReadInt( curitem );
                byte [ ] unconvertedstr = war3proc.ReadMem( curitem + 0x30 , 4 );
                Array.Reverse( unconvertedstr );
                string curitemid = Encoding.UTF8.GetString( unconvertedstr );
                bool ignore = false;
                foreach ( string ignoreid in ignoreclasslist )
                {
                    if ( curitemid == ignoreid )
                    {
                        ignore = true;
                    }
                }

                if ( !ignore )
                {
                    DataStr tmpstr = new DataStr( );
                    tmpstr.ID = curitemid;
                    bool foundor = false;
                    for ( int n = 0 ; n < items.Count ; n++ )
                    {
                        if ( items [ n ].ID == curitemid )
                        {
                            tmpstr.count = items [ n ].count + 1;
                            items [ n ] = tmpstr;
                            foundor = true;
                        }
                    }
                    if ( !foundor )
                    {
                        tmpstr.count = 1;
                        items.Add( tmpstr );
                    }
                }
            }




            label3.Text = unitscount.ToString( );
            label4.Text = itemscount.ToString( );

            for ( int i = 0 ; i < units.Count ; i++ )
            {
                string tmp = "Class:" + units [ i ].ID + ".  Count:" + units [ i ].count + ".";
                listBox1.Items.Add( tmp );
            }

            for ( int i = 0 ; i < items.Count ; i++ )
            {
                string tmp = "Class:" + items [ i ].ID + ".  Count:" + items [ i ].count + ".";
                listBox2.Items.Add( tmp );
            }

        }

        private void button1_Click ( object sender , EventArgs e )
        {
           
        }

        private void button2_Click ( object sender , EventArgs e )
        {
            ignoreclasslist.Add( textBox1.Text );
        }

        private void button3_Click ( object sender , EventArgs e )
        {
            for ( int i = 0 ; i < listBox1.Items.Count ; i++ )
            {
                string str = ( string ) listBox1.Items [ i ];
                Match regm = Regex.Match( str , @"Class:(.*?)\." );
                if ( regm.Success )
                {
                    ignoreclasslist.Add( regm.Groups [ 1 ].Value );
                }
            }

            for ( int i = 0 ; i < listBox2.Items.Count ; i++ )
            {
                string str = ( string ) listBox2.Items [ i ];
                Match regm = Regex.Match( str , @"Class:(.*?)\." );
                if ( regm.Success )
                {
                    ignoreclasslist.Add( regm.Groups [ 1 ].Value );
                }
            }
        }

        private void timer1_Tick ( object sender , EventArgs e )
        {
            try
            {
                ReXfresh( );
            }
            catch
            {

            }
        }
    }
}
