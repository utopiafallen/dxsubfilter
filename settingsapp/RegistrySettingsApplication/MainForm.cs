using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace RegistrySettingsApplication
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
        }

        private void BufferCountTextBox_TextChanged(object sender, EventArgs e)
        {
            String temptext = BufferCountTextBox.Text;
            int tempint = -1;
            try
            {
                tempint = Convert.ToInt32(temptext);
                BufferCountTextBoxToolTip.Hide(BufferCountTextBox);
            }
            catch (FormatException)
            {
                BufferCountTextBoxToolTip.Show("That is not a number", BufferCountTextBox);
            }
            catch (OverflowException)
            {
                BufferCountTextBoxToolTip.Show("That number is too large", BufferCountTextBox);
            }
        }

        private void DefScreenPosTextBox_TextChanged(object sender, EventArgs e)
        {
            String temptext = DefScreenPosTextBox.Text;
            int tempint = -1;
            try
            {
                tempint = Convert.ToInt32(temptext);
                DefScreenPosTextBoxToolTip.Hide(DefScreenPosTextBox);
            }
            catch (FormatException)
            {
                DefScreenPosTextBoxToolTip.Show("That is not a number", DefScreenPosTextBox);
            }
            catch (OverflowException)
            {
                DefScreenPosTextBoxToolTip.Show("That number is too large", DefScreenPosTextBox);
            }
        }

        private void LeftMarginTextBox_TextChanged(object sender, EventArgs e)
        {
            String temptext = LeftMarginTextBox.Text;
            int tempint = -1;
            try
            {
                tempint = Convert.ToInt32(temptext);
                LeftMarginTextBoxToolTip.Hide(LeftMarginTextBox);
            }
            catch (FormatException)
            {
                LeftMarginTextBoxToolTip.Show("That is not a number", LeftMarginTextBox);
            }
            catch (OverflowException)
            {
                LeftMarginTextBoxToolTip.Show("That number is too large", LeftMarginTextBox);
            }
        }

        private void RightMarginTextBox_TextChanged(object sender, EventArgs e)
        {
            String temptext = RightMarginTextBox.Text;
            int tempint = -1;
            try
            {
                tempint = Convert.ToInt32(temptext);
                RightMarginTextBoxToolTip.Hide(RightMarginTextBox);
            }
            catch (FormatException)
            {
                RightMarginTextBoxToolTip.Show("That is not a number", RightMarginTextBox);
            }
            catch (OverflowException)
            {
                RightMarginTextBoxToolTip.Show("That number is too large", RightMarginTextBox);
            }
        }

        private void TopMarginTextBox_TextChanged(object sender, EventArgs e)
        {
            String temptext = TopMarginTextBox.Text;
            int tempint = -1;
            try
            {
                tempint = Convert.ToInt32(temptext);
                TopMarginTextBoxToolTip.Hide(TopMarginTextBox);
            }
            catch (FormatException)
            {
                TopMarginTextBoxToolTip.Show("That is not a number", TopMarginTextBox);
            }
            catch (OverflowException)
            {
                TopMarginTextBoxToolTip.Show("That number is too large", TopMarginTextBox);
            }
        }

        private void BottomMarginTextBox_TextChanged(object sender, EventArgs e)
        {
            String temptext = BottomMarginTextBox.Text;
            int tempint = -1;
            try
            {
                tempint = Convert.ToInt32(temptext);
                BottomMarginTextBoxToolTip.Hide(BottomMarginTextBox);
            }
            catch (FormatException)
            {
                BottomMarginTextBoxToolTip.Show("That is not a number", BottomMarginTextBox);
            }
            catch (OverflowException)
            {
                BottomMarginTextBoxToolTip.Show("That number is too large", BottomMarginTextBox);
            }
        }


    }
}
