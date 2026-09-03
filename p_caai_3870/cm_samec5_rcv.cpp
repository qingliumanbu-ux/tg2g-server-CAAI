/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:
Version:    1.0
Date:     2019-8-21
Description: 月总费用明细接收信息
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"
#include "epex.h" 
BM2F_ENTERACE_TELE(cm_samec5_rcv)

int f_cm_samec5_rcv(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CModel tcaaia11("TCAAIA11");
	int doFlag = 0;
	CString sqlstr = " ";
	try
	{
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tcaaia11.Reset();
			//tcaaia11.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			tcaaia11["PROD_DATE"] = bcls_rec->Tables[0].Rows[i]["BUDAT"].ToString().Trim();  
			tcaaia11["COST_CENTER"] = bcls_rec->Tables[0].Rows[i]["RCNTR"].ToString().Trim();
			tcaaia11["COST_CENTER_CNAME"] = bcls_rec->Tables[0].Rows[i]["LTEXT"].ToString().Trim();
			tcaaia11["MAT_CODE"] = bcls_rec->Tables[0].Rows[i]["RACCT"].ToString().Trim();
			tcaaia11["MAT_NAME"] = bcls_rec->Tables[0].Rows[i]["TXT50"].ToString().Trim();
			tcaaia11["AMT"] = bcls_rec->Tables[0].Rows[i]["HSL"].ToDecimal();
			tcaaia11["ACCOUNT_PERIOD"] = bcls_rec->Tables[0].Rows[i]["ACCOUNT_PERIOD"].ToString().Trim();

			tcaaia11["REC_CREATE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
			tcaaia11["REC_CREATOR"] = s.userid;
			tcaaia11.TrimOrBlank(); //去除多余的空格，如果是null的话字符型赋值空格或是数字小赋0
			tcaaia11.Delete("ACCOUNT_PERIOD,COST_CENTER,MAT_CODE");
			tcaaia11.Insert();
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


