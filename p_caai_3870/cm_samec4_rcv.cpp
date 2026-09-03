/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:
Version:    1.0
Date:     2019-8-21
Description: bom 信息接收信息
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
#include "epex.h" 


BM2F_ENTERACE_TELE(cm_samec4_rcv)

int f_cm_samec4_rcv(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CModel tcaac05c("TCAAC05C");
	int doFlag = 0;
	CString action_code = " ";
	CString sqlstr = " ";

	CDbCommand cmd_inq(conn);
	try
	{
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tcaac05c.Reset();
			tcaac05c.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			tcaac05c["KEY_SEQ"] = bcls_rec->Tables[0].Rows[i]["SEQ_NO"].ToString().TrimOrBlank();

			tcaac05c["REC_CREATE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
			tcaac05c["REC_CREATOR"] = s.userid;
			tcaac05c.TrimOrBlank(); //去除多余的空格，如果是null的话字符型赋值空格或是数字小赋0
			if (bcls_rec->Tables[0].Rows[i]["MSG_TYPE"].ToString() == "D")
			{
				tcaac05c.Delete("PRODUCT_CODE,KEY_SEQ");
			}
			else
			{	
				//取牌号、标准
				sqlstr = " select sg_sign"
					"  from TQMTPA4"
					" where SG_SEQ = @sg_seq"
					;
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("sg_seq", tcaac05c["PRODUCT_CODE"].ToString().SubstringNE(6,3));
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					tcaac05c["SG_SIGN"] = cmd_inq.GetString(1);
				}
				cmd_inq.Close();
				
				sqlstr = " select SG_STD"
					"  from TQMTPA5"
					" where std_code = @std_code"
					;
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("std_code", tcaac05c["PRODUCT_CODE"].ToString().SubstringNE(2,4));
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					tcaac05c["SG_STD"] = cmd_inq.GetString(1);
				}
				cmd_inq.Close();
				tcaac05c.Delete("PRODUCT_CODE,KEY_SEQ");
				tcaac05c.Insert();
			}
		}
	}
	catch (CDbException& ex)
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}],请联系开发人员", arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;
	}
	catch (CApplicationException& ex)
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strcpy(s.msg, ex.GetMsg());
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}


